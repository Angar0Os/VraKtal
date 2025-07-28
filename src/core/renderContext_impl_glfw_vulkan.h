#ifndef VRAKTAL_CORE_RENDER_CONTEXT_IMPL_GLFW_VULKAN_H
#define VRAKTAL_CORE_RENDER_CONTEXT_IMPL_GLFW_VULKAN_H
#pragma once

#include <core/renderContext.h>

#include "../vkb/VkBootstrap.h"
#include <vma/vk_mem_alloc.h>

#include "../vkTypes.h"

#include <iostream>
#include <GLFW/glfw3.h>	

struct core::rhi::RenderContext::Internal
{
	GLFWwindow* window;
	vkb::Instance instance;
	VkSurfaceKHR surface;
	VkDebugUtilsMessengerEXT debugMessenger;

	bool useValidationLayers = false;

	VkDevice device;
	VkPhysicalDevice chosenGPU;
	VmaAllocator allocator;	
	VkQueue graphicsQueue;
	uint32_t graphicsQueueFamily;

	VkFence immFence;
	VkCommandBuffer immCommandBuffer;
	VkCommandPool immCommandPool;

	VkExtent2D windowExtent;

	VkSwapchainKHR swapchain;
	VkFormat swapchainImageFormat;
	VkExtent2D swapchainExtent;
	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;

	vkTypes::AllocatedImage drawImage;
	vkTypes::AllocatedImage depthImage;

	void CreateSwapchain(uint32_t width, uint32_t height);
	VkImageCreateInfo ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
	VkImageViewCreateInfo ImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
};

#endif //VRAKTAL_CORE_RENDER_CONTEXT_IMPL_GLFW_VULKAN_H
