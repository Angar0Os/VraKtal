#include <vk/sync.h>

using namespace vk;

VkFenceCreateInfo VulkanSync::FenceCreateInfo(VkFenceCreateFlags flags)
{
	VkFenceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	info.pNext = nullptr;

	info.flags = flags;

	return info;
}

VkSemaphoreCreateInfo VulkanSync::SemaphoreCreateInfo(VkSemaphoreCreateFlags flags)
{
	VkSemaphoreCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	info.pNext = nullptr;

	info.flags = flags;

	return info;
}

void VulkanSync::Init(VulkanCommandBuffer commandBuffer, VulkanDevice device)
{
	VkFenceCreateInfo fenceCreateInfo = FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	VkSemaphoreCreateInfo semaphoreCreateInfo = SemaphoreCreateInfo();

	for (int i = 0; i < FRAME_OVERLAP; ++i)
	{
		vkCreateFence(device.GetVkDevice(), &fenceCreateInfo, nullptr, &commandBuffer._frames[i]._renderFence);
		vkCreateSemaphore(device.GetVkDevice(), &semaphoreCreateInfo, nullptr, &commandBuffer._frames[i]._swapchainSemaphore);
		vkCreateSemaphore(device.GetVkDevice(), &semaphoreCreateInfo, nullptr, &commandBuffer._frames[i]._renderSemaphore);
	}

	vkCreateFence(device.GetVkDevice(), &fenceCreateInfo, nullptr, &commandBuffer._immFence);
	commandBuffer.GetDeletionQueue().push_function([=]() { vkDestroyFence(device.GetVkDevice(), commandBuffer._immFence, nullptr); });
}