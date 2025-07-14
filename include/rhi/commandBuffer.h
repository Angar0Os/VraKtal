#ifndef VRAKTAL_RHI_COMMAND_BUFFER_H
#define VRAKTAL_RHI_COMMAND_BUFFER_H
#pragma once

#include <functional>
#include <../../src/vkb/VkBootstrap.h>

namespace rhi
{
	class CommandBuffer
	{
	public:
		virtual ~CommandBuffer() noexcept = default;

		virtual void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function) = 0;
		virtual VkCommandBufferBeginInfo BeginInfo(VkCommandBufferUsageFlags flags) = 0;
		virtual VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags) = 0;
		virtual VkCommandBufferAllocateInfo AllocateInfo(VkCommandPool pool, uint32_t count) = 0;
		virtual VkCommandBufferSubmitInfo CommandBufferSubmitInfo(VkCommandBuffer cmd) = 0;
		virtual void Init() = 0;
	};
}

#endif // VRAKTAL_RHI_COMMAND_BUFFER_H
