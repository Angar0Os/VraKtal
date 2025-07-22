#include "commandBuffer_impl_vulkan.h"

using namespace core::gpu::rhi;

core::gpu::rhi::CommandBuffer::CommandBuffer()
{

}

void CommandBuffer::Internal::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function)
{
}