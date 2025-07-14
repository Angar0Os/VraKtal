#ifndef VRAKTAL_VK_SYNC_H
#define VRAKTAL_VK_SYNC_H
#pragma once

#include <rhi/sync.h>

#include <vk/commandBuffer.h>
#include <vk/device.h>

namespace vk
{
	class VulkanSync : public rhi::Sync
	{
	public:
		VulkanSync() = default;

		VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags) override;
		VkSemaphoreCreateInfo SemaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0) override;
		void Init(VulkanCommandBuffer commandBuffer, VulkanDevice device);
	};
}

#endif //VRAKTAL_VK_SYNC_H
