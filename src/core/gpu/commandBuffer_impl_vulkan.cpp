#include "commandBuffer_impl_vulkan.h"



using namespace core::gpu::rhi;

CommandBuffer::CommandBuffer(core::rhi::RenderContext& rCtx)
	: m_Internal(std::make_unique<Internal>(rCtx))
{

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