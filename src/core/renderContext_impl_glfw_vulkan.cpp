#include "renderContext_impl_glfw_vulkan.h"
#include "gpu/descriptor_impl_vulkan.h"

#include <GLFW/glfw3.h>	

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

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
		glfwTerminate();
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, descriptor.resizeable ? GLFW_TRUE : GLFW_FALSE);
	m_Internal->window = glfwCreateWindow(descriptor.windowSize.x, descriptor.windowSize.y, descriptor.windowTitle, nullptr, nullptr);

	if (!m_Internal->window)
	{
		throw std::runtime_error("Failed to create GLFW window");
		glfwTerminate();
	}

	vkb::InstanceBuilder builder;

	auto inst_ret = builder.set_app_name(descriptor.windowTitle)
		.request_validation_layers(m_Internal->useValidationLayers)
		.use_default_debug_messenger()
		.require_api_version(1, 3, 0)
		.build();

	vkb::Instance vkb_inst = inst_ret.value();

	m_Internal->instance.instance = vkb_inst.instance;
	m_Internal->debugMessenger = vkb_inst.debug_messenger;

	glfwCreateWindowSurface(m_Internal->instance, m_Internal->window, nullptr, &m_Internal->surface);

	VkPhysicalDeviceVulkan13Features features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
	features.dynamicRendering = true;
	features.synchronization2 = true;

	vkb::PhysicalDeviceSelector selector{ m_Internal->instance };
	vkb::PhysicalDevice physicalDevice = selector
		.set_minimum_version(1, 3)
		.set_required_features_13(features)
		.set_surface(m_Internal->surface)
		.select()
		.value();

	vkb::DeviceBuilder deviceBuilder{ physicalDevice };
	vkb::Device vkbDevice = deviceBuilder.build().value();

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

	m_Internal->commandBuffer = std::make_unique<core::rhi::gpu::CommandBuffer>(*this);

	m_Internal->CreateSwapchain(m_Internal->windowExtent.width, m_Internal->windowExtent.height);

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

	VkImageCreateInfo rimg_info = m_Internal->ImageCreateInfo(m_Internal->drawImage.imageFormat, drawImageUsages, drawImageExtent);

	VmaAllocationCreateInfo rimg_allocinfo = {};
	rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vmaCreateImage(m_Internal->allocator, &rimg_info, &rimg_allocinfo, &m_Internal->drawImage.image, &m_Internal->drawImage.allocation, nullptr);

	VkImageViewCreateInfo rview_info = m_Internal->ImageViewCreateInfo(m_Internal->drawImage.imageFormat, m_Internal->drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

	vkCreateImageView(m_Internal->device, &rview_info, nullptr, &m_Internal->drawImage.imageView);

	m_Internal->depthImage.imageFormat = VK_FORMAT_D32_SFLOAT;
	m_Internal->depthImage.imageExtent = drawImageExtent;

	VkImageUsageFlags depthImageUsages{};
	depthImageUsages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	VkImageCreateInfo dimg_info = m_Internal->ImageCreateInfo(m_Internal->depthImage.imageFormat, depthImageUsages, drawImageExtent);

	vmaCreateImage(m_Internal->allocator, &dimg_info, &rimg_allocinfo, &m_Internal->depthImage.image, &m_Internal->depthImage.allocation, nullptr);
	VkImageViewCreateInfo dview_info = m_Internal->ImageViewCreateInfo(m_Internal->depthImage.imageFormat, m_Internal->depthImage.image, VK_IMAGE_ASPECT_DEPTH_BIT);

	vkCreateImageView(m_Internal->device, &dview_info, nullptr, &m_Internal->depthImage.imageView);

	VkDevice device = m_Internal->device;
	VmaAllocator allocator = m_Internal->allocator;
	VkImageView drawImageView = m_Internal->drawImage.imageView;
	VkImage drawImageHandle = m_Internal->drawImage.image;
	VmaAllocation drawImageAllocation = m_Internal->drawImage.allocation;
	VkImageView depthImageView = m_Internal->depthImage.imageView;
	VkImage depthImageHandle = m_Internal->depthImage.image;
	VmaAllocation depthImageAllocation = m_Internal->depthImage.allocation;

	m_Internal->commandBuffer->GetInternal().mainDeletionQueue.PushFunction([=]() {
		vkDestroyImageView(device, drawImageView, nullptr);
		vmaDestroyImage(allocator, drawImageHandle, drawImageAllocation);

		vkDestroyImageView(device, depthImageView, nullptr);
		vmaDestroyImage(allocator, depthImageHandle, depthImageAllocation);
		});

	m_Internal->commandBuffer->GetInternal().InitCommand();

	VkFenceCreateInfo fenceCreateInfo = m_Internal->FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	vkCreateFence(m_Internal->device, &fenceCreateInfo, nullptr, &m_Internal->immFence);

	m_Internal->commandBuffer->GetInternal().mainDeletionQueue.PushFunction([=]() { vkDestroyFence(m_Internal->device, m_Internal->immFence, nullptr); });

	for (int i = 0; i < m_Internal->FRAME_OVERLAP; ++i)
	{
		vkCreateFence(m_Internal->device, &fenceCreateInfo, nullptr, &m_Internal->frames[i].renderFence);

		VkSemaphoreCreateInfo semaphoreCreateInfo = m_Internal->SemaphoreCreateInfo();

		vkCreateSemaphore(m_Internal->device, &semaphoreCreateInfo, nullptr, &m_Internal->frames[i].swapchainSemaphore);
		vkCreateSemaphore(m_Internal->device, &semaphoreCreateInfo, nullptr, &m_Internal->frames[i].renderSemaphore);

		m_Internal->commandBuffer->GetInternal().mainDeletionQueue.PushFunction([=]() {
			vkDestroyFence(m_Internal->device, m_Internal->frames[i].renderFence, nullptr);
			vkDestroySemaphore(m_Internal->device, m_Internal->frames[i].swapchainSemaphore, nullptr);
			vkDestroySemaphore(m_Internal->device, m_Internal->frames[i].renderSemaphore, nullptr);
			});
	}

	m_Internal->descriptor->GetInternal().InitDescriptor(*this);
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

VkImageCreateInfo RenderContext::Internal::ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent)
{
	VkImageCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	info.pNext = nullptr;

	info.imageType = VK_IMAGE_TYPE_2D;

	info.format = format;
	info.extent = extent;

	info.mipLevels = 1;
	info.arrayLayers = 1;

	info.samples = VK_SAMPLE_COUNT_1_BIT;

	info.tiling = VK_IMAGE_TILING_OPTIMAL;
	info.usage = usageFlags;

	return info;
}

VkImageViewCreateInfo RenderContext::Internal::ImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	info.pNext = nullptr;

	info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	info.image = image;
	info.format = format;
	info.subresourceRange.baseMipLevel = 0;
	info.subresourceRange.levelCount = 1;
	info.subresourceRange.baseArrayLayer = 0;
	info.subresourceRange.layerCount = 1;
	info.subresourceRange.aspectMask = aspectFlags;

	return info;
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

VkFenceCreateInfo RenderContext::Internal::FenceCreateInfo(VkFenceCreateFlags flags)
{
	VkFenceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	info.pNext = nullptr;

	info.flags = flags;

	return info;
}

VkSemaphoreCreateInfo RenderContext::Internal::SemaphoreCreateInfo(VkSemaphoreCreateFlags flags)
{
	VkSemaphoreCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	info.pNext = nullptr;

	info.flags = flags;

	return info;
}
