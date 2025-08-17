#ifndef VRAKTAL_CORE_GPU_COMMAND_BUFFER_IMPL_VULKAN_H
#define VRAKTAL_CORE_GPU_COMMAND_BUFFER_IMPL_VULKAN_H
#pragma once

#include <core/gpu/commandBuffer.h>

#include "../gpu-details/vkTypes.h"

#include "../renderContext_impl_glfw_vulkan.h"
#include "../../vkb/VkBootstrap.h"

struct core::rhi::gpu::CommandBuffer::Internal
{
	Internal(core::rhi::RenderContext& ctx) : renderContext(ctx) {}

	core::rhi::RenderContext& renderContext;
	vkTypes::DeletionQueue mainDeletionQueue;	

	void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);
	void InitCommand();
};

#endif //VRAKTAL_CORE_GPU_COMMAND_BUFFER_IMPL_VULKAN_H
