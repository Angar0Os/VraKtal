#include "renderer_impl_vulkan.h"

using namespace rhi::core;
using namespace rhi::core::gpu;

RendererVulkan::RendererVulkan(Pipeline* pipeline)
    : m_pipeline(pipeline)
{
}

void RendererVulkan::Render(CommandBuffer& commandBuffer, const RenderingInfo& info, uint32_t imageIndex)
{
    commandBuffer.BeginRendering(info, imageIndex);
    commandBuffer.BindPipeline(m_pipeline);
    commandBuffer.Draw(3, info.width, info.height);
    commandBuffer.EndRendering(imageIndex);
}