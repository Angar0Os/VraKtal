#ifndef VRAKTAL_CORE_GPU_COMMAND_BUFFER_IMPL_VULKAN_H
#define VRAKTAL_CORE_GPU_COMMAND_BUFFER_IMPL_VULKAN_H
#pragma once

#include <core/gpu/commandBuffer.h>

#include "../renderContext_impl_glfw_vulkan.h"
#include "../../vkb/VkBootstrap.h"

struct core::gpu::rhi::CommandBuffer::Internal
{
	Internal(core::rhi::RenderContext& ctx) : renderContext(ctx) {}

	core::rhi::RenderContext& renderContext;
	DeletionQueue mainDeletionQueue;

	void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);
	VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags);
};

#endif //VRAKTAL_CORE_GPU_COMMAND_BUFFER_IMPL_VULKAN_H
