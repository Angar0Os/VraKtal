#ifndef VRAKTAL_GRAPHICS_RENDERER_IMPL_VULKAN_H
#define VRAKTAL_GRAPHICS_RENDERER_IMPL_VULKAN_H
#pragma once

#include <core/gpu/renderer.h>
#include <core/gpu/pipeline.h>

namespace rhi::core::gpu
{
    class RendererVulkan final : public Renderer
    {
    public:
        RendererVulkan(Pipeline* pipeline);
        void Render(CommandBuffer& commandBuffer, const RenderingInfo& info, uint32_t imageIndex) override;
    private:
        Pipeline* m_pipeline;
    }; 
};

#endif //VRAKTAL_GRAPHICS_RENDERER_IMPL_VULKAN_H
