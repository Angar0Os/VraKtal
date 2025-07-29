#ifndef VRAKTAL_CORE_RENDER_CONTEXT_IMPL_GLFW_VULKAN_H
#define VRAKTAL_CORE_RENDER_CONTEXT_IMPL_GLFW_VULKAN_H
#pragma once

#include <core/renderContext.h>

#include "../vkb/VkBootstrap.h"
#include <vma/vk_mem_alloc.h>

#include "gpu/commandBuffer_impl_vulkan.h"
#include "../vkTypes.h"

#include <iostream>
#include <GLFW/glfw3.h>	

struct FrameData
{
	VkSemaphore swapchainSemaphore, renderSemaphore;
	VkFence renderFence;

	VkCommandPool commandPool;
	VkCommandBuffer mainCommandBuffer;

	vkTypes::DeletionQueue deletionQueue;
	vkTypes::DescriptorAllocatorGrowable frameDescriptors;
};

struct core::rhi::RenderContext::Internal
{
	Internal(RenderContext* parent);
	~Internal() = default;

	GLFWwindow* window;

	vkb::Instance instance;
	VkSurfaceKHR surface;
	VkDebugUtilsMessengerEXT debugMessenger;

	std::unique_ptr<core::gpu::rhi::CommandBuffer> commandBuffer;

	bool useValidationLayers = false;

	static constexpr unsigned int FRAME_OVERLAP = 2;

	int frameNumber{ 0 };

	VkDevice device;
	VkPhysicalDevice chosenGPU;
	VmaAllocator allocator;
	VkQueue graphicsQueue;
	uint32_t graphicsQueueFamily;

	VkFence immFence;
	VkCommandBuffer immCommandBuffer;
	VkCommandPool immCommandPool;

	VkExtent2D windowExtent;

	FrameData frames[FRAME_OVERLAP];

	VkSwapchainKHR swapchain;
	VkFormat swapchainImageFormat;
	VkExtent2D swapchainExtent;
	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;

	vkTypes::AllocatedImage drawImage;
	vkTypes::AllocatedImage depthImage;

	FrameData& GetCurrentFrame();
	FrameData& GetLastFrame();

	void CreateSwapchain(uint32_t width, uint32_t height);
	VkImageCreateInfo ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
	VkImageViewCreateInfo ImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
	VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags);
	VkSemaphoreCreateInfo SemaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);

private:
	RenderContext* m_parent;
};

#endif //VRAKTAL_CORE_RENDER_CONTEXT_IMPL_GLFW_VULKAN_H