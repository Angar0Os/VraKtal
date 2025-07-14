#ifndef VRAKTAL_VK_COMMANDBUFFER_H
#define VRAKTAL_VK_COMMANDBUFFER_H
#pragma once

#include <rhi/commandBuffer.h>

namespace vk
{
	class VulkanCommandBuffer : public rhi::CommandBuffer
	{
	public:
		VulkanCommandBuffer() = default;

		VulkanCommandBuffer(VkFence immFence, VkCommandBuffer immCommandBuffer, VkDevice device, VkQueue graphicsQueue)
			: _immFence(immFence), _immCommandBuffer(immCommandBuffer), _device(device), _graphicsQueue(graphicsQueue) {
		}

		void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function) override;
		VkCommandBufferBeginInfo BeginInfo(VkCommandBufferUsageFlags flags) override;
		VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags) override;
		VkCommandBufferAllocateInfo AllocateInfo(VkCommandPool pool, uint32_t count) override;
		VkCommandBufferSubmitInfo CommandBufferSubmitInfo(VkCommandBuffer cmd) override;
		VkSubmitInfo2 SubmitInfo(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo);
	private:
		VkFence _immFence;
		VkCommandBuffer _immCommandBuffer;
		VkDevice _device;
		VkQueue _graphicsQueue;
	};
}

#endif //VRAKTAL_VK_COMMANDBUFFER_H
