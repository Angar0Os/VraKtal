#ifndef VRAKTAL_CORE_RENDER_CONTEXT_IMPL_GLFW_VULKAN_H
#define VRAKTAL_CORE_RENDER_CONTEXT_IMPL_GLFW_VULKAN_H
#pragma once

#include <core/renderContext.h>

#include "../vkb/VkBootstrap.h"
#include <vma/vk_mem_alloc.h>

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
};

#endif //VRAKTAL_CORE_RENDER_CONTEXT_IMPL_GLFW_VULKAN_H
