#ifndef VRAKTAL_VK_ENGINE_H
#define VRAKTAL_VK_ENGINE_H
#pragma once

#include "vkb/VkBootstrap.h"

#include <core/renderContext.h>

class VulkanEngine
{
public:
	explicit VulkanEngine();
	~VulkanEngine() noexcept;
	
	void init();
	void run();
	void cleanup();

	vk::core::RenderContext rCtx;
};

#endif //VRAKTAL_VK_ENGINE_H