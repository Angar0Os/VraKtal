#ifndef VRAKTAL_VK_ENGINE_H
#define VRAKTAL_VK_ENGINE_H
#pragma once

#include "vkb/VkBootstrap.h"

#include <vk/instance.h>

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
	core::RenderContext rCtx;

	VkInstance instance;
	VkDebugUtilsMessengerEXT debug_messenger;
	VkAllocationCallbacks* callBacks;
	VkSurfaceKHR surface;

	bool useValidationLayers = true;
};

#endif //VRAKTAL_VK_ENGINE_H