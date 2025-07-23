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

void CommandBuffer::Internal::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function)
{
	vkResetFences(renderContext.GetInternal().device, 1, &renderContext.GetInternal().immFence);
	vkResetCommandBuffer(renderContext.GetInternal().immCommandBuffer, 0);

	VkCommandBuffer cmd = renderContext.GetInternal().immCommandBuffer;

	VkCommandBufferBeginInfo cmdBeginInfo = CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
}