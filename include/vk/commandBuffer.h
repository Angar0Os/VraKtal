#ifndef VRAKTAL_VK_COMMANDBUFFER_H
#define VRAKTAL_VK_COMMANDBUFFER_H
#pragma once

#include <rhi/commandBuffer.h>

#include <queue>
#include <functional>


namespace vk
{
	struct DeletionQueue
	{
		std::deque<std::function<void()>> deletors;

		void push_function(std::function<void()>&& function)
		{
			deletors.push_back(function);
		}

		void flush()
		{
			for (auto it = deletors.rbegin(); it != deletors.rend(); ++it)
			{
				(*it)();
			}

			deletors.clear();
		}
	};

	struct FrameData
	{
		VkSemaphore _swapchainSemaphore, _renderSemaphore;
		VkFence _renderFence;

		VkCommandPool _commandPool;
		VkCommandBuffer _mainCommandBuffer;

		DeletionQueue _deletionQueue;
		//DescriptorAllocatorGrowable _frameDescriptors; TODO : Implement this
	};

	constexpr unsigned int FRAME_OVERLAP = 2;

	class VulkanCommandBuffer : public rhi::CommandBuffer
	{
	public:
		VulkanCommandBuffer() = default;

		VulkanCommandBuffer(VkFence immFence, VkCommandBuffer immCommandBuffer, VkDevice device, VkQueue graphicsQueue)
			: _immFence(immFence), _immCommandBuffer(immCommandBuffer), _device(device), _graphicsQueue(graphicsQueue) {
		}

		unsigned int _frameNumber = 0;

		FrameData _frames[FRAME_OVERLAP];
		FrameData& get_current_frame() { return _frames[_frameNumber % FRAME_OVERLAP]; };

		void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function) override;
		void Init() override;
		VkCommandBufferBeginInfo BeginInfo(VkCommandBufferUsageFlags flags) override;
		VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags) override;
		VkCommandBufferSubmitInfo CommandBufferSubmitInfo(VkCommandBuffer cmd) override;
		VkCommandBufferAllocateInfo AllocateInfo(VkCommandPool pool, uint32_t count) override;
		VkSubmitInfo2 SubmitInfo(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo);

		DeletionQueue& GetDeletionQueue() { return _mainDeletionQueue; }
	private:
		DeletionQueue _mainDeletionQueue;

		VkFence _immFence;
		VkCommandBuffer _immCommandBuffer;
		VkCommandPool _immCommandPool;

		VkDevice _device;
		VkQueue _graphicsQueue;
		uint32_t _graphicsQueueFamily;
	};
}

#endif //VRAKTAL_VK_COMMANDBUFFER_H
