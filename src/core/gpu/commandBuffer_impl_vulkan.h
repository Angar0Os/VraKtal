#ifndef VRAKTAL_CORE_GPU_COMMAND_BUFFER_IMPL_VULKAN_H
#define VRAKTAL_CORE_GPU_COMMAND_BUFFER_IMPL_VULKAN_H
#pragma once

#include <core/gpu/commandBuffer.h>

#include "../../vkb/VkBootstrap.h"

//struct FrameData
//{
//	VkSemaphore _swapchainSemaphore, _renderSemaphore;
//	VkFence _renderFence;
//
//	VkCommandPool _commandPool;
//	VkCommandBuffer _mainCommandBuffer;
//
//	core::gpu::rhi::DeletionQueue _deletionQueue;
//	DescriptorAllocatorGrowable _frameDescriptors;
//};

struct core::gpu::rhi::CommandBuffer::Internal
{
	DeletionQueue mainDeletionQueue;

	void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);
};

#endif //VRAKTAL_CORE_GPU_COMMAND_BUFFER_IMPL_VULKAN_H
