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

