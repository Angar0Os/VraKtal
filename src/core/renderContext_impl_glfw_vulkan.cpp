#include "renderContext_impl_glfw_vulkan.h"

#include "gpu/descriptor_impl_vulkan.h"
#include "gpu/pipeline_impl_vulkan.h"
#include "gpu/image_impl_vulkan.h"

#include "../graphics/material_impl_vulkan.h"

#include <GLFW/glfw3.h>

#define VMA_IMPLEMENTATION
#include <chrono>
#include <vma/vk_mem_alloc.h>

#include <../tracy/public/tracy/Tracy.hpp>

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

	m_Internal->commandBuffer = std::make_unique<core::rhi::gpu::CommandBuffer>(*this);
	m_Internal->pipeline = std::make_unique<core::rhi::gpu::Pipeline>();
	m_Internal->material = std::make_unique<graphics::rhi::Material>();
	m_Internal->meshLoader = std::make_unique<graphics::rhi::Mesh>();
	m_Internal->imageLoader = std::make_unique<core::rhi::Image>();
	m_Internal->descriptor = std::make_unique<core::rhi::gpu::Descriptor>();

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

	auto error = vmaCreateImage(m_Internal->allocator, &rimg_info, &rimg_allocinfo, &m_Internal->drawImage.image, &m_Internal->drawImage.allocation, nullptr);

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

	m_Internal->CreateSwapchain(m_Internal->windowExtent.width, m_Internal->windowExtent.height);

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
	m_Internal->pipeline->GetInternal().InitBackgroundPipelines(*this);
	m_Internal->pipeline->GetInternal().InitBackgroundPipelines(*this);
	m_Internal->material->GetInternal().metalRoughMaterial.BuildPipelines(*this);

	m_Internal->InitDefaultData();
	m_Internal->InitRenderable();

	m_Internal->IsInitialized = true;

	demo::InputManager::getInstance().Initialize(m_Internal->window);

	m_Internal->camera = new demo::Camera(m_Internal->window);
	m_Internal->camera->velocity = glm::vec3(0.0f);
	m_Internal->camera->position = glm::vec3(30.0f, -00.f, -085.f);
	m_Internal->camera->pitch = 0;
	m_Internal->camera->yaw = 0;
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

void RenderContext::Internal::InitDefaultData()
{
	std::array<Vertex, 4> rect_vertices;

	rect_vertices[0].position = { 0.5,-0.5, 0 };
	rect_vertices[1].position = { 0.5,0.5, 0 };
	rect_vertices[2].position = { -0.5,-0.5, 0 };
	rect_vertices[3].position = { -0.5,0.5, 0 };

	rect_vertices[0].color = { 0,0, 0,1 };
	rect_vertices[1].color = { 0.5,0.5,0.5 ,1 };
	rect_vertices[2].color = { 1,0, 0,1 };
	rect_vertices[3].color = { 0,1, 0,1 };

	rect_vertices[0].uv_x = 1;
	rect_vertices[0].uv_y = 0;
	rect_vertices[1].uv_x = 0;
	rect_vertices[1].uv_y = 0;
	rect_vertices[2].uv_x = 1;
	rect_vertices[2].uv_y = 1;
	rect_vertices[3].uv_x = 0;
	rect_vertices[3].uv_y = 1;

	std::array<uint32_t, 6> rect_indices;

	rect_indices[0] = 0;
	rect_indices[1] = 1;
	rect_indices[2] = 2;

	rect_indices[3] = 2;
	rect_indices[4] = 1;
	rect_indices[5] = 3;
	
	rectangle = meshLoader->GetInternal().UploadMesh(rect_indices, rect_vertices, *m_parent);

	uint32_t white = glm::packUnorm4x8(glm::vec4(1, 1, 1, 1));
	whiteImage = imageLoader->GetInternal().CreateImage((void*)&white, VkExtent3D{ 1, 1, 1}, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, *m_parent);

	uint32_t grey = glm::packUnorm4x8(glm::vec4(0.66f, 0.66f, 0.66f, 1));
	greyImage = imageLoader->GetInternal().CreateImage((void*)&grey, VkExtent3D{ 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, *m_parent);

	uint32_t black = glm::packUnorm4x8(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	blackImage = imageLoader->GetInternal().CreateImage((void*)&white, VkExtent3D{ 1, 1, 1}, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, *m_parent);

	uint32_t magenta = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));
	std::array<uint32_t, 16 * 16 > pixels;

	for (int x = 0; x < 16; ++x)
	{
		for (int y = 0; y < 16; ++y)
		{
			pixels[y * 16 + x] = ((x % 2) ^ (y % 2)) ? magenta : black;
		}
	}

	errorCheckerboardImage = imageLoader->GetInternal().CreateImage(pixels.data(), VkExtent3D{ 16, 16, 1 }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, *m_parent);

	VkSamplerCreateInfo sampl = { .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };

	sampl.magFilter = VK_FILTER_NEAREST;
	sampl.minFilter = VK_FILTER_NEAREST;

	vkCreateSampler(device, &sampl, nullptr, &defaultSamplerNearest);

	sampl.magFilter = VK_FILTER_LINEAR;
	sampl.minFilter = VK_FILTER_LINEAR;
	vkCreateSampler(device, &sampl, nullptr, &defaultSamplerLinear);
}

void RenderContext::Internal::InitRenderable()
{
	std::string structurePath = { "../bin/assets/meshes/house.glb" };
	auto structureFile = meshLoader->GetInternal().LoadGLTF(m_parent, structurePath);

	if (!structureFile)
		throw std::runtime_error(structureFile.error());

	LoadedScenes["structure"] = *structureFile;
}

void RenderContext::Internal::Run()
{
	bool bQuit = false;
	int frameCount = 0;

	while (!bQuit && !glfwWindowShouldClose(window))
	{
		auto start = std::chrono::system_clock::now();

		glfwPollEvents();

		if (glfwWindowShouldClose(window))
		{
			bQuit = true;
		}

		if (ResizeRequested)
		{
			ResizeSwapchain();
		}

		if (glfwGetWindowAttrib(window, GLFW_ICONIFIED))
		{
			FreezeRendering = true;
		}
		else {
			FreezeRendering = false;
		}

		if (FreezeRendering) continue;

		frameCount++;

		UpdateScene();
		Draw();

		auto end = std::chrono::system_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	}
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

void RenderContext::Internal::UpdateScene()
{
	if (!camera)
	{
		std::cerr << "Camera is null!" << std::endl;
		return;
	}

	camera->Update(frameNumber);

	glm::mat4 view = camera->GetViewMatrix();

	glm::mat4 projection = glm::perspective(
		glm::radians(70.0f),
		(float)windowExtent.width / (float)windowExtent.height,
		0.1f,
		10000.f
	);

	projection[1][1] *= -1;

	sceneData.view = view;
	sceneData.proj = projection;
	sceneData.viewproj = projection * view;

	auto sceneIt = LoadedScenes.find("structure");
	if (sceneIt != LoadedScenes.end() && sceneIt->second)
	{
		sceneIt->second->Draw(glm::mat4{ 1.f }, drawCommands);
	}
	else
	{
		std::cerr << "Structure scene not found or null!" << std::endl;
	}
}

VkSemaphoreSubmitInfo RenderContext::Internal::SemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore)
{
	VkSemaphoreSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.semaphore = semaphore;
	submitInfo.stageMask = stageMask;
	submitInfo.deviceIndex = 0;
	submitInfo.value = 1;

	return submitInfo;
}

VkPresentInfoKHR RenderContext::Internal::PresentInfo()
{
	VkPresentInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	info.pNext = 0;

	info.swapchainCount = 0;
	info.pSwapchains = nullptr;
	info.pWaitSemaphores = nullptr;
	info.waitSemaphoreCount = 0;
	info.pImageIndices = nullptr;

	return info;
}

VkRenderingAttachmentInfo RenderContext::Internal::DepthAttachmentInfo(VkImageView view, VkImageLayout layout)
{
	VkRenderingAttachmentInfo depthAttachment{};
	depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	depthAttachment.pNext = nullptr;

	depthAttachment.imageView = view;
	depthAttachment.imageLayout = layout;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.clearValue.depthStencil.depth = 0.f;

	return depthAttachment;
}

VkRenderingAttachmentInfo RenderContext::Internal::AttachmentInfo(VkImageView view, VkClearValue* clear, VkImageLayout layout)
{
	VkRenderingAttachmentInfo colorAttachment{};
	colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorAttachment.pNext = nullptr;

	colorAttachment.imageView = view;
	colorAttachment.imageLayout = layout;
	colorAttachment.loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	if (clear) {
		colorAttachment.clearValue = *clear;
	}

	return colorAttachment;
}

void RenderContext::Internal::Draw()
{
	if (!IsInitialized)
	{
		std::cerr << "Engine not initialized!" << std::endl;
		return;
	}

	vkWaitForFences(device, 1, &GetCurrentFrame().renderFence, true, 1000000000);

	GetCurrentFrame().deletionQueue.Flush();
	GetCurrentFrame().frameDescriptors->ClearPools(device);

	uint32_t swapchainImageIndex;

	VkResult e = vkAcquireNextImageKHR(device, swapchain, 1000000000, GetCurrentFrame().swapchainSemaphore, nullptr, &swapchainImageIndex);
	if (e == VK_ERROR_OUT_OF_DATE_KHR)
	{
		ResizeRequested = true;
		return;
	}

	if (e != VK_SUCCESS && e != VK_SUBOPTIMAL_KHR)
	{
		std::cerr << "Failed to acquire swapchain image: " << e << std::endl;
		return;
	}

	drawExtent.height = std::min(swapchainExtent.height, drawImage.imageExtent.height) * renderScale;
	drawExtent.width = std::min(swapchainExtent.width, drawImage.imageExtent.width) * renderScale;

	if (drawExtent.width == 0 || drawExtent.height == 0)
	{
		std::cerr << "Invalid draw extent!" << std::endl;
		return;
	}

	vkResetFences(device, 1, &GetCurrentFrame().renderFence);
	vkResetCommandBuffer(GetCurrentFrame().mainCommandBuffer, 0);

	VkCommandBuffer cmd = GetCurrentFrame().mainCommandBuffer;
	VkCommandBufferBeginInfo cmdBeginInfo = commandBuffer->GetInternal().CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkResult beginResult = vkBeginCommandBuffer(cmd, &cmdBeginInfo);
	if (beginResult != VK_SUCCESS)
	{
		std::cerr << "Failed to begin command buffer: " << beginResult << std::endl;
		return;
	}

	imageLoader->GetInternal().TransitionImage(cmd, drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
	imageLoader->GetInternal().TransitionImage(cmd, depthImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

	DrawMain(cmd);

	imageLoader->GetInternal().TransitionImage(cmd, drawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	imageLoader->GetInternal().TransitionImage(cmd, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	VkExtent2D extent;
	extent.height = windowExtent.height;
	extent.width = windowExtent.width;

	imageLoader->GetInternal().CopyImageToImage(cmd, drawImage.image, swapchainImages[swapchainImageIndex], drawExtent, swapchainExtent);

	imageLoader->GetInternal().TransitionImage(cmd, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	imageLoader->GetInternal().TransitionImage(cmd, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	vkEndCommandBuffer(cmd);

	VkCommandBufferSubmitInfo cmdInfo = commandBuffer->GetInternal().CommandBufferSubmitInfo(cmd);

	VkSemaphoreSubmitInfo waitInfo = SemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, GetCurrentFrame().swapchainSemaphore);
	VkSemaphoreSubmitInfo signalInfo = SemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, GetCurrentFrame().renderSemaphore);

	VkSubmitInfo2 submit = commandBuffer->GetInternal().SubmitInfo(&cmdInfo, &signalInfo, &waitInfo);
	vkQueueSubmit2(graphicsQueue, 1, &submit, GetCurrentFrame().renderFence);

	VkPresentInfoKHR presentInfo = PresentInfo();
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pWaitSemaphores = &GetCurrentFrame().renderSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &swapchainImageIndex;

	VkResult presentResult = vkQueuePresentKHR(graphicsQueue, &presentInfo);
	if (e == VK_ERROR_OUT_OF_DATE_KHR)
	{
		ResizeRequested = true;
		return;
	}

	frameNumber++;
	FrameMark;
}

VkRenderingInfo	RenderContext::Internal::RenderingInfo(VkExtent2D renderExtent, VkRenderingAttachmentInfo* colorAttachment, VkRenderingAttachmentInfo* depthAttachment)
{
	VkRenderingInfo renderInfo{};
	renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderInfo.pNext = nullptr;

	renderInfo.renderArea = VkRect2D{ VkOffset2D { 0, 0 }, renderExtent };
	renderInfo.layerCount = 1;
	renderInfo.colorAttachmentCount = 1;
	renderInfo.pColorAttachments = colorAttachment;
	renderInfo.pDepthAttachment = depthAttachment;
	renderInfo.pStencilAttachment = nullptr;

	return renderInfo;
}

void RenderContext::Internal::DrawMain(VkCommandBuffer cmd)
{
	ComputeEffect& effect = pipeline->GetInternal().backgroundEffects[currentBackgroundEffect];

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, effect.pipeline);

	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetInternal().gradientPipelineLayout, 0, 1, &descriptor->GetInternal().drawImageDescriptors, 0, nullptr);

	vkCmdPushConstants(cmd, pipeline->GetInternal().gradientPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePushConstants), &effect.data);
	vkCmdDispatch(cmd, std::ceil(windowExtent.width / 16.0f), std::ceil(windowExtent.height / 16.0f), 1);

	imageLoader->GetInternal().TransitionImage(cmd, drawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	VkRenderingAttachmentInfo colorAttachment = AttachmentInfo(drawImage.imageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	VkRenderingAttachmentInfo depthAttachment = DepthAttachmentInfo(depthImage.imageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
	depthAttachment.clearValue.depthStencil = { 1.0f, 0 };
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

	VkRenderingInfo renderInfo = RenderingInfo(windowExtent, &colorAttachment, &depthAttachment);

	vkCmdBeginRendering(cmd, &renderInfo);
	auto start = std::chrono::system_clock::now();
	DrawGeometry(cmd);

	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	vkCmdEndRendering(cmd);
}

bool IsVisible(const RenderObject& obj, const glm::mat4& viewproj)
{
	std::array<glm::vec3, 8> corners{
		glm::vec3 { 1, 1, 1 },
		glm::vec3 { 1, 1, -1 },
		glm::vec3 { 1, -1, 1 },
		glm::vec3 { 1, -1, -1 },
		glm::vec3 { -1, 1, 1 },
		glm::vec3 { -1, 1, -1 },
		glm::vec3 { -1, -1, 1 },
		glm::vec3 { -1, -1, -1 },
	};

	glm::mat4 matrix = viewproj * obj.transform;

	glm::vec3 min = { 1.5, 1.5, 1.5 };
	glm::vec3 max = { -1.5, -1.5, -1.5 };

	for (int c = 0; c < 8; c++)
	{
		glm::vec4 v = matrix * glm::vec4(obj.bounds.origin + (corners[c] * obj.bounds.extents), 1.f);

		v.x = v.x / v.w;
		v.y = v.y / v.w;
		v.z = v.z / v.w;

		min = glm::min(glm::vec3{ v.x, v.y, v.z }, min);
		max = glm::max(glm::vec3{ v.x, v.y, v.z }, max);
	}

	if (min.z > 1.f || max.z < 0.f || min.x > 1.f || max.x < -1.f || min.y > 1.f || max.y < -1.f)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void RenderContext::Internal::DrawGeometry(VkCommandBuffer cmd)
{
	std::vector<uint32_t> opaque_draws;
	opaque_draws.reserve(drawCommands.OpaqueSurfaces.size());

	for (int i = 0; i < drawCommands.OpaqueSurfaces.size(); ++i)
	{
		if (IsVisible(drawCommands.OpaqueSurfaces[i], sceneData.viewproj))
		{
			opaque_draws.push_back(i);
		}
	}

	std::sort(opaque_draws.begin(), opaque_draws.end(), [&](const auto& iA, const auto& iB) {
		const RenderObject& A = drawCommands.OpaqueSurfaces[iA];
		const RenderObject& B = drawCommands.OpaqueSurfaces[iB];
		if (A.material == B.material) {
			return A.indexBuffer < B.indexBuffer;
		}
		else {
			return A.material < B.material;
		}
		});

	vkTypes::AllocatedBuffer gpuSceneDataBuffer = commandBuffer->GetInternal().CreateBuffer(sizeof(vkTypes::GPUSceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	GetCurrentFrame().deletionQueue.PushFunction([=, this]() {
		commandBuffer->GetInternal().DestroyBuffer(gpuSceneDataBuffer);
		});

	vkTypes::GPUSceneData* sceneUniformData = (vkTypes::GPUSceneData*)gpuSceneDataBuffer.allocation->GetMappedData();
	*sceneUniformData = sceneData;

	VkDescriptorSet globalDescriptor = GetCurrentFrame().frameDescriptors->Allocate(device, descriptor->GetInternal().gpuSceneDataDescriptorLayout);

	DescriptorWriter writer;
	writer.WriteBuffer(0, gpuSceneDataBuffer.buffer, sizeof(vkTypes::GPUSceneData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	writer.UpdateSet(device, globalDescriptor);

	MaterialPipeline* lastPipeline = nullptr;
	MaterialInstance* lastMaterial = nullptr;
	VkBuffer lastIndexBuffer = VK_NULL_HANDLE;

	auto draw = [&](const RenderObject& r) {
		if (r.material != lastMaterial) {
			lastMaterial = r.material;
			if (r.material->pipeline != lastPipeline) {

				lastPipeline = r.material->pipeline;
				vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, r.material->pipeline->pipeline);
				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, r.material->pipeline->layout, 0, 1,
					&globalDescriptor, 0, nullptr);

				VkViewport viewport = {};
				viewport.x = 0;
				viewport.y = 0;
				viewport.width = (float)windowExtent.width;
				viewport.height = (float)windowExtent.height;
				viewport.minDepth = 0.f;
				viewport.maxDepth = 1.f;

				vkCmdSetViewport(cmd, 0, 1, &viewport);

				VkRect2D scissor = {};
				scissor.offset.x = 0;
				scissor.offset.y = 0;
				scissor.extent.width = windowExtent.width;
				scissor.extent.height = windowExtent.height;

				vkCmdSetScissor(cmd, 0, 1, &scissor);
			}

			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, r.material->pipeline->layout, 1, 1,
				&r.material->materialSet, 0, nullptr);
		}
		if (r.indexBuffer != lastIndexBuffer) {
			lastIndexBuffer = r.indexBuffer;
			vkCmdBindIndexBuffer(cmd, r.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		}

		GPUDrawPushConstants push_constants;
		push_constants.worldMatrix = r.transform;
		push_constants.vertexBuffer = r.vertexBufferAddress;

		vkCmdPushConstants(cmd, r.material->pipeline->layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &push_constants);
		vkCmdDrawIndexed(cmd, r.indexCount, 1, r.firstIndex, 0, 0);
		};

	for (auto& r : opaque_draws)
	{
		draw(drawCommands.OpaqueSurfaces[r]);
	}

	for (auto& r : drawCommands.TransparentSurfaces)
	{
		draw(r);
	}

	drawCommands.OpaqueSurfaces.clear();
	drawCommands.TransparentSurfaces.clear();
}
