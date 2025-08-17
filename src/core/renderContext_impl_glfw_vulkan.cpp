#include "renderContext_impl_glfw_vulkan.h"

#include "gpu/descriptor_impl_vulkan.h"
#include "gpu/pipeline_impl_vulkan.h"
#include "gpu/image_impl_vulkan.h"

#include "../graphics/material_impl_vulkan.h"

#include <GLFW/glfw3.h>

#define VMA_IMPLEMENTATION
#include <chrono>
#include <iostream>
#include <vma/vk_mem_alloc.h>

#include <../tracy/public/tracy/Tracy.hpp>

#include "core/gpu/commandBuffer.h"
#include "gpu-details/vkInitializers.h"

#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "vulkan-1.lib")

using namespace core::rhi;

RenderContext::Internal::Internal(RenderContext* parent)
	: m_parent(parent)
{
}

RenderContext::RenderContext(const RenderContextDescriptor& descriptor)
	: m_Internal(std::make_unique<Internal>(this))
{

	if (!glfwInit())
	{
		throw std::runtime_error("Failed to initialize GLFW");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, descriptor.resizeable ? GLFW_TRUE : GLFW_FALSE);
	m_Internal->window = glfwCreateWindow(descriptor.windowSize.x, descriptor.windowSize.y, descriptor.windowTitle, nullptr, nullptr);

	if (!m_Internal->window)
	{
		throw std::runtime_error("Failed to create GLFW window");
	}

	m_Internal->windowExtent = { descriptor.windowSize.x, descriptor.windowSize.y };

	vkb::InstanceBuilder builder;

	auto inst_ret = builder.set_app_name("VraKtal")
		.request_validation_layers(m_Internal->useValidationLayers)
		.use_default_debug_messenger()
		.require_api_version(1, 3, 0)
		.build();

	if (!inst_ret) {
		throw std::runtime_error("Failed to create Vulkan instance: " + inst_ret.error().message());
	}

	vkb::Instance vkb_inst = inst_ret.value();

	m_Internal->instance = vkb_inst;
	m_Internal->debugMessenger = vkb_inst.debug_messenger;

	VkResult result = glfwCreateWindowSurface(m_Internal->instance.instance, m_Internal->window, nullptr, &m_Internal->surface);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create window surface! Error code: " + std::to_string(result));
	}

	VkPhysicalDeviceVulkan13Features features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
	features.dynamicRendering = true;
	features.synchronization2 = true;

	vkb::PhysicalDeviceSelector selector{ m_Internal->instance };
	auto physicalDeviceRet = selector
		.set_minimum_version(1, 3)
		.set_required_features_13(features)
		.set_surface(m_Internal->surface)
		.prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
		.allow_any_gpu_device_type(false)
		.select();

	if (!physicalDeviceRet) {
		throw std::runtime_error("Failed to select physical device: " + physicalDeviceRet.error().message());
	}

	vkb::PhysicalDevice physicalDevice = physicalDeviceRet.value();

	vkb::DeviceBuilder deviceBuilder{ physicalDevice };
	auto vkbDeviceRet = deviceBuilder.build();

	if (!vkbDeviceRet) {
		throw std::runtime_error("Failed to create logical device: " + vkbDeviceRet.error().message());
	}

	vkb::Device vkbDevice = vkbDeviceRet.value();

	m_Internal->device = vkbDevice.device;
	m_Internal->chosenGPU = physicalDevice.physical_device;

	m_Internal->graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
	m_Internal->graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = m_Internal->chosenGPU;
	allocatorInfo.device = m_Internal->device;
	allocatorInfo.instance = m_Internal->instance;
	allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	vmaCreateAllocator(&allocatorInfo, &m_Internal->allocator);

	VkExtent3D drawImageExtent = {
		m_Internal->windowExtent.width,
		m_Internal->windowExtent.height,
		1
	};

	m_Internal->drawImage.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
	m_Internal->drawImage.imageExtent = drawImageExtent;

	VkImageUsageFlags drawImageUsages{};
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	VkImageCreateInfo rimg_info = gpu_detail::ImageCreateInfo(m_Internal->drawImage.imageFormat, drawImageUsages, drawImageExtent);

	VmaAllocationCreateInfo rimg_allocinfo = {};
	rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	auto error = vmaCreateImage(m_Internal->allocator, &rimg_info, &rimg_allocinfo, &m_Internal->drawImage.image, &m_Internal->drawImage.allocation, nullptr);

	VkImageViewCreateInfo rview_info = gpu_detail::ImageViewCreateInfo(m_Internal->drawImage.imageFormat, m_Internal->drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

	vkCreateImageView(m_Internal->device, &rview_info, nullptr, &m_Internal->drawImage.imageView);

	m_Internal->depthImage.imageFormat = VK_FORMAT_D32_SFLOAT;
	m_Internal->depthImage.imageExtent = drawImageExtent;

	VkImageUsageFlags depthImageUsages{};
	depthImageUsages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	VkImageCreateInfo dimg_info = gpu_detail::ImageCreateInfo(m_Internal->depthImage.imageFormat, depthImageUsages, drawImageExtent);

	vmaCreateImage(m_Internal->allocator, &dimg_info, &rimg_allocinfo, &m_Internal->depthImage.image, &m_Internal->depthImage.allocation, nullptr);
	VkImageViewCreateInfo dview_info = gpu_detail::ImageViewCreateInfo(m_Internal->depthImage.imageFormat, m_Internal->depthImage.image, VK_IMAGE_ASPECT_DEPTH_BIT);

	vkCreateImageView(m_Internal->device, &dview_info, nullptr, &m_Internal->depthImage.imageView);

	m_Internal->CreateSwapchain(m_Internal->windowExtent.width, m_Internal->windowExtent.height);

	VkFenceCreateInfo fenceCreateInfo = gpu_detail::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	
	for (int i = 0; i < m_Internal->FRAME_OVERLAP; ++i)
	{
		vkCreateFence(m_Internal->device, &fenceCreateInfo, nullptr, &m_Internal->frames[i].renderFence);

		VkSemaphoreCreateInfo semaphoreCreateInfo = gpu_detail::SemaphoreCreateInfo();

		vkCreateSemaphore(m_Internal->device, &semaphoreCreateInfo, nullptr, &m_Internal->frames[i].swapchainSemaphore);
		vkCreateSemaphore(m_Internal->device, &semaphoreCreateInfo, nullptr, &m_Internal->frames[i].renderSemaphore);

		m_Internal->mainDeletionQueue.PushFunction([=]() {
			vkDestroyFence(m_Internal->device, m_Internal->frames[i].renderFence, nullptr);
			vkDestroySemaphore(m_Internal->device, m_Internal->frames[i].swapchainSemaphore, nullptr);
			vkDestroySemaphore(m_Internal->device, m_Internal->frames[i].renderSemaphore, nullptr);
			});
	}

	m_Internal->IsInitialized = true;
}

void RenderContext::Internal::CreateSwapchain(uint32_t width, uint32_t height)
{
	vkb::SwapchainBuilder swapchainBuilder{ chosenGPU, device, surface };

	swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

	vkb::Swapchain vkbSwapchain = swapchainBuilder
		.set_desired_format(VkSurfaceFormatKHR{ .format = swapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.set_desired_extent(width, height)
		.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		.build()
		.value();

	swapchainExtent = vkbSwapchain.extent;
	swapchain = vkbSwapchain.swapchain;
	swapchainImages = vkbSwapchain.get_images().value();
	swapchainImageViews = vkbSwapchain.get_image_views().value();
}



FrameData& RenderContext::Internal::GetCurrentFrame()
{
	return frames[frameNumber % FRAME_OVERLAP];
}

FrameData& RenderContext::Internal::GetLastFrame()
{
	return frames[(frameNumber - 1) % FRAME_OVERLAP];
}

RenderContext::~RenderContext()
{

}

void RenderContext::Internal::ResizeSwapchain()
{
	vkDeviceWaitIdle(device);

	DestroySwapchain();

	int w, h;
	glfwGetWindowSize(window, &w, &h);
	windowExtent.width = w;
	windowExtent.height = h;

	CreateSwapchain(windowExtent.width, windowExtent.height);

	ResizeRequested = false;
}

void RenderContext::Internal::DestroySwapchain()
{
	vkDestroySwapchainKHR(device, swapchain, nullptr);

	for (int i = 0; i < swapchainImageViews.size(); ++i)
	{
		vkDestroyImageView(device, swapchainImageViews[i], nullptr);
	}
}

void RenderContext::BeginFrame()
{
	vkWaitForFences(m_Internal->device, 1, &m_Internal->GetCurrentFrame().renderFence, true, 1000000000);

	m_Internal->GetCurrentFrame().deletionQueue.Flush();
	m_Internal->GetCurrentFrame().frameDescriptors->ClearPools(m_Internal->device);

	VkResult e = vkAcquireNextImageKHR(m_Internal->device, m_Internal->swapchain, 1000000000, m_Internal->GetCurrentFrame().swapchainSemaphore, nullptr, &m_Internal->swapchainImageIndex);
	if (e == VK_ERROR_OUT_OF_DATE_KHR)
	{
		m_Internal->ResizeRequested = true;
		return;
	}

	if (e != VK_SUCCESS && e != VK_SUBOPTIMAL_KHR)
		std::cerr << "Failed to acquire swapchain image: " << e << std::endl;
}

bool RenderContext::Present()
{
	VkSemaphoreSubmitInfo waitInfo = gpu_detail::SemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, m_Internal->GetCurrentFrame().swapchainSemaphore);
	VkSemaphoreSubmitInfo signalInfo = gpu_detail::SemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, m_Internal->GetCurrentFrame().renderSemaphore);

	VkCommandBufferSubmitInfo cmdInfo = gpu_detail::CommandBufferSubmitInfo(m_Internal->GetCurrentFrame().mainCommandBuffer);
	VkSubmitInfo2 submit = gpu_detail::SubmitInfo(&cmdInfo, { signalInfo }, { waitInfo });
	vkQueueSubmit2(m_Internal->graphicsQueue, 1, &submit, m_Internal->GetCurrentFrame().renderFence);

	VkPresentInfoKHR presentInfo = gpu_detail::PresentInfo(
		m_Internal->swapchain,
		m_Internal->GetCurrentFrame().renderSemaphore,
		m_Internal->swapchainImageIndex
	);

	VkResult presentResult = vkQueuePresentKHR(m_Internal->graphicsQueue, &presentInfo);
	if (presentResult == VK_ERROR_OUT_OF_DATE_KHR)
		m_Internal->ResizeRequested = true;

	m_Internal->frameNumber++;
	FrameMark;

	return !glfwWindowShouldClose(m_Internal->window);
}

vkTypes::AllocatedBuffer RenderContext::Internal::CreateBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	VkBufferCreateInfo bufferInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;
	bufferInfo.size = allocSize;

	bufferInfo.usage = usage;

	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage = memoryUsage;
	vmaallocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
	vkTypes::AllocatedBuffer newBuffer;

	vmaCreateBuffer(allocator, &bufferInfo, &vmaallocInfo, &newBuffer.buffer, &newBuffer.allocation, &newBuffer.info);

	return newBuffer;
}

void RenderContext::Internal::DestroyBuffer(const vkTypes::AllocatedBuffer buffer)
{
	vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
}