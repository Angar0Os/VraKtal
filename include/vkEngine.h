#ifndef VRAKTAL_VK_ENGINE_H
#define VRAKTAL_VK_ENGINE_H
#pragma once

#include "vkb/VkBootstrap.h"

#include <vk/instance.h>
#include <vk/device.h>
#include <vk/swapchain.h>
#include <vk/commandBuffer.h>
#include <vk/sync.h>
#include <vk/descriptors.h>

#include <core/renderContext.h>


class VulkanEngine
{
public:
	explicit VulkanEngine();
	~VulkanEngine() noexcept;
	
	void init();
	void run();
	void cleanup();

	vk::Instance vkInstance;
	vk::VulkanSwapchain vkSwapchain;
	vk::VulkanDevice vkDevice;
	vk::VulkanCommandBuffer vkCommandBuffer;
	vk::VulkanSync vkSync;
	vk::VulkanDescriptor vkDescriptor;

	core::RenderContext rCtx;

	vkb::Instance instance;
	VkDebugUtilsMessengerEXT debug_messenger;
	VkAllocationCallbacks* callBacks;
	VkSurfaceKHR surface;

	VkFence immFence;
	VkCommandBuffer immCommandBuffer;

	bool useValidationLayers = true;
};

#endif //VRAKTAL_VK_ENGINE_H