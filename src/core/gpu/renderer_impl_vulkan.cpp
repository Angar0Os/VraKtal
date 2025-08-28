#include "../src/core/gpu/renderer_impl_vulkan.h"

using namespace rhi::core;
using namespace rhi::core::gpu;

TriangleRenderer::TriangleRenderer(Pipeline* pipeline)
    : m_pipeline(pipeline)
{
}

void TriangleRenderer::Render(CommandBuffer& commandBuffer, const RenderingInfo& info)
{
    commandBuffer.BeginRendering(info);
    commandBuffer.BindPipeline(m_pipeline);
    commandBuffer.Draw(3);
    commandBuffer.EndRendering();
}