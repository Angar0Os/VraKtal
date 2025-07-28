#include "commandBuffer_impl_vulkan.h"



using namespace core::gpu::rhi;

CommandBuffer::CommandBuffer(core::rhi::RenderContext& rCtx)
	: m_Internal(std::make_unique<Internal>(rCtx))
{

}

void CommandBuffer::Internal::InitCommand()
{
	VkCommandPoolCreateInfo commandPoolInfo = CommandPoolCreateInfo(renderContext.GetInternal().graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	for (int i = 0; i < renderContext.GetInternal().FRAME_OVERLAP; ++i)
	{
		vkCreateCommandPool(renderContext.GetInternal().device, &commandPoolInfo, nullptr, &renderContext.GetInternal().frames[i].commandPool);
		VkCommandBufferAllocateInfo cmdAllocInfo = CommandBufferAllocateInfo(renderContext.GetInternal().frames[i].commandPool, 1);

		vkAllocateCommandBuffers(renderContext.GetInternal().device, &cmdAllocInfo, &renderContext.GetInternal().frames[i].mainCommandBuffer);
		mainDeletionQueue.PushFunction([=]() { vkDestroyCommandPool(renderContext.GetInternal().device, renderContext.GetInternal().frames[i].commandPool, nullptr); });
	}

	vkCreateCommandPool(renderContext.GetInternal().device, &commandPoolInfo, nullptr, &renderContext.GetInternal().immCommandPool);
	VkCommandBufferAllocateInfo cmdAllocInfo = CommandBufferAllocateInfo(renderContext.GetInternal().immCommandPool, 1);

	vkAllocateCommandBuffers(renderContext.GetInternal().device, &cmdAllocInfo, &renderContext.GetInternal().immCommandBuffer);
	mainDeletionQueue.PushFunction([=]() {vkDestroyCommandPool(renderContext.GetInternal().device, renderContext.GetInternal().immCommandPool, nullptr); });
}

VkCommandBufferAllocateInfo CommandBuffer::Internal::CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count)
{
	VkCommandBufferAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.pNext = nullptr;

	info.commandPool = pool;
	info.commandBufferCount = count;
	info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	return info;
}

VkCommandBufferBeginInfo CommandBuffer::Internal::CommandBufferBeginInfo(VkCommandBufferUsageFlags flags)
{
	VkCommandBufferBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.pNext = nullptr;

	info.pInheritanceInfo = nullptr;
	info.flags = flags;
	return info;
}

VkCommandBufferSubmitInfo CommandBuffer::Internal::CommandBufferSubmitInfo(VkCommandBuffer cmd)
{
	VkCommandBufferSubmitInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
	info.pNext = nullptr;
	info.commandBuffer = cmd;
	info.deviceMask = 0;

	return info;
}


VkSubmitInfo2 CommandBuffer::Internal::SubmitInfo(VkCommandBufferSubmitInfo * cmd, VkSemaphoreSubmitInfo * signalSemaphoreInfo, VkSemaphoreSubmitInfo * waitSemaphoreInfo)
{
	VkSubmitInfo2 info = {};
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	info.pNext = nullptr;

	info.waitSemaphoreInfoCount = waitSemaphoreInfo == nullptr ? 0 : 1;
	info.pWaitSemaphoreInfos = waitSemaphoreInfo;

	info.signalSemaphoreInfoCount = signalSemaphoreInfo == nullptr ? 0 : 1;
	info.pSignalSemaphoreInfos = signalSemaphoreInfo;

	info.commandBufferInfoCount = 1;
	info.pCommandBufferInfos = cmd;

	return info;
}

void CommandBuffer::Internal::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function)
{
	vkResetFences(renderContext.GetInternal().device, 1, &renderContext.GetInternal().immFence);
	vkResetCommandBuffer(renderContext.GetInternal().immCommandBuffer, 0);

	VkCommandBuffer cmd = renderContext.GetInternal().immCommandBuffer;

	VkCommandBufferBeginInfo cmdBeginInfo = CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	vkBeginCommandBuffer(cmd, &cmdBeginInfo);

	function(cmd);

	vkEndCommandBuffer(cmd);

	VkCommandBufferSubmitInfo cmdInfo = CommandBufferSubmitInfo(cmd);
	VkSubmitInfo2 submit = SubmitInfo(&cmdInfo, nullptr, nullptr);

	vkQueueSubmit2(renderContext.GetInternal().graphicsQueue, 1, &submit, renderContext.GetInternal().immFence);
	vkWaitForFences(renderContext.GetInternal().device, 1, &renderContext.GetInternal().immFence, true, 9999999999);
}

VkCommandPoolCreateInfo CommandBuffer::Internal::CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags)
{
	VkCommandPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.pNext = nullptr;
	info.queueFamilyIndex = queueFamilyIndex;
	info.flags = flags;
	return info;
}