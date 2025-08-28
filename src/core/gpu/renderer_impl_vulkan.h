#ifndef VRAKTAL_RHI_CORE_GPU_RENDERER_VULKAN_H
#define VRAKTAL_RHI_CORE_GPU_RENDERER_VULKAN_H
#pragma once

#include <core/gpu/renderer.h>
#include <core/gpu/pipeline.h>

namespace rhi::core::gpu
{
    class TriangleRenderer final : public Renderer
    {
    public:
        TriangleRenderer(Pipeline* pipeline);

        void Render(CommandBuffer& commandBuffer, const RenderingInfo& info) override;

    private:
        Pipeline* m_pipeline;
    }; 
}

#endif //VRAKTAL_RHI_CORE_GPU_RENDERER_VULKAN_H