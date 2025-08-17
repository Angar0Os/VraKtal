#include "commandBuffer_impl_vulkan.h"

#include "../gpu-details/vkInitializers.h"


using namespace core::rhi::gpu;

CommandBuffer::CommandBuffer(core::rhi::RenderContext& rCtx)
	: m_Internal(std::make_unique<Internal>(rCtx))
{

}

void CommandBuffer::Internal::InitCommand()
{
	VkCommandPoolCreateInfo commandPoolInfo = gpu_detail::CommandPoolCreateInfo(renderContext.GetInternal().graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	for (int i = 0; i < renderContext.GetInternal().FRAME_OVERLAP; ++i)
	{
		vkCreateCommandPool(renderContext.GetInternal().device, &commandPoolInfo, nullptr, &renderContext.GetInternal().frames[i].commandPool);
		VkCommandBufferAllocateInfo cmdAllocInfo = gpu_detail::CommandBufferAllocateInfo(renderContext.GetInternal().frames[i].commandPool, 1);

		vkAllocateCommandBuffers(renderContext.GetInternal().device, &cmdAllocInfo, &renderContext.GetInternal().frames[i].mainCommandBuffer);
		mainDeletionQueue.PushFunction([=]() { vkDestroyCommandPool(renderContext.GetInternal().device, renderContext.GetInternal().frames[i].commandPool, nullptr); });
	}

	vkCreateCommandPool(renderContext.GetInternal().device, &commandPoolInfo, nullptr, &renderContext.GetInternal().immCommandPool);
	VkCommandBufferAllocateInfo cmdAllocInfo = gpu_detail::CommandBufferAllocateInfo(renderContext.GetInternal().immCommandPool, 1);

	vkAllocateCommandBuffers(renderContext.GetInternal().device, &cmdAllocInfo, &renderContext.GetInternal().immCommandBuffer);
	mainDeletionQueue.PushFunction([=]() {vkDestroyCommandPool(renderContext.GetInternal().device, renderContext.GetInternal().immCommandPool, nullptr); });
}

void CommandBuffer::Internal::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function)
{
	vkResetFences(renderContext.GetInternal().device, 1, &renderContext.GetInternal().immFence);
	vkResetCommandBuffer(renderContext.GetInternal().immCommandBuffer, 0);

	VkCommandBuffer cmd = renderContext.GetInternal().immCommandBuffer;

	VkCommandBufferBeginInfo cmdBeginInfo = gpu_detail::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	vkBeginCommandBuffer(cmd, &cmdBeginInfo);

	function(cmd);

	vkEndCommandBuffer(cmd);

	VkCommandBufferSubmitInfo cmdInfo = gpu_detail::CommandBufferSubmitInfo(cmd);
	VkSubmitInfo2 submit = gpu_detail::SubmitInfo(&cmdInfo, {}, {});

	vkQueueSubmit2(renderContext.GetInternal().graphicsQueue, 1, &submit, renderContext.GetInternal().immFence);
	vkWaitForFences(renderContext.GetInternal().device, 1, &renderContext.GetInternal().immFence, true, 9999999999);
}

