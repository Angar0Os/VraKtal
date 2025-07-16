#ifndef VRAKTAL_VK_ENGINE_H
#define VRAKTAL_VK_ENGINE_H
#pragma once

#include "../../src/vkb/VkBootstrap.h"

#include <vk/instance.h>
#include <vk/device.h>
#include <vk/swapchain.h>
#include <vk/commandBuffer.h>
#include <vk/sync.h>
#include <vk/descriptors.h>
#include <vk/material.h>
#include <vk/pipeline.h>

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
	vk::GLTFMetallic_Roughness vkMetalRoughMaterial;
	vk::VulkanPipeline vkPipeline;

	vk::AllocatedImage* _whiteImage;
	vk::AllocatedImage* _blackImage;
	vk::AllocatedImage* _greyImage;
	vk::AllocatedImage* _errorCheckerboardImage;

	core::RenderContext rCtx;

	vkb::Instance instance;
	VkDebugUtilsMessengerEXT debug_messenger;
	VkAllocationCallbacks* callBacks;
	VkSurfaceKHR surface;

	VkFence immFence;
	VkCommandBuffer immCommandBuffer;


	bool useValidationLayers = true;

	static VulkanEngine& Get();
};

#endif //VRAKTAL_VK_ENGINE_H