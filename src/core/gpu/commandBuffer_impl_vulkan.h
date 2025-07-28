#ifndef VRAKTAL_CORE_GPU_COMMAND_BUFFER_IMPL_VULKAN_H
#define VRAKTAL_CORE_GPU_COMMAND_BUFFER_IMPL_VULKAN_H
#pragma once

#include <core/gpu/commandBuffer.h>

#include "../../vkTypes.h"

#include "../renderContext_impl_glfw_vulkan.h"
#include "../../vkb/VkBootstrap.h"

struct core::gpu::rhi::CommandBuffer::Internal
{
	Internal(core::rhi::RenderContext& ctx) : renderContext(ctx) {}

	core::rhi::RenderContext& renderContext;
	vkTypes::DeletionQueue mainDeletionQueue;	

	void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);
	void InitCommand();

	VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count);
	VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags);
	VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags);
	VkCommandBufferSubmitInfo CommandBufferSubmitInfo(VkCommandBuffer cmd);
	VkSubmitInfo2 SubmitInfo(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo);
};

#endif //VRAKTAL_CORE_GPU_COMMAND_BUFFER_IMPL_VULKAN_H
