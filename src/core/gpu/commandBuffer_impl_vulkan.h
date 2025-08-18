#ifndef VRAKTAL_CORE_GPU_COMMAND_BUFFER_IMPL_VULKAN_H
#define VRAKTAL_CORE_GPU_COMMAND_BUFFER_IMPL_VULKAN_H
#pragma once

#include <core/gpu/commandBuffer.h>
#include "../gpu-details/vkTypes.h"
#include "../renderContext_impl_glfw_vulkan.h"

struct core::rhi::gpu::CommandBuffer::Internal
{
	Internal(RenderContext& ctx) : renderContext(ctx) {}

	RenderContext& renderContext;
	vkTypes::DeletionQueue mainDeletionQueue;
	
	void InitCommand();
};

#endif //VRAKTAL_CORE_GPU_COMMAND_BUFFER_IMPL_VULKAN_H
