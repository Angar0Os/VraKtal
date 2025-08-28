#ifndef VRAKTAL_RHI_CORE_GPU_RENDERER.H
#define VRAKTAL_RHI_CORE_GPU_RENDERER_H
#pragma once

#include <core/gpu/commandBuffer.h>
#include <core/gpu/renderingInfo.h>

namespace rhi::core::gpu
{
    class Renderer
    {
    public:
        virtual ~Renderer() = default;
        virtual void Render(gpu::CommandBuffer& cmd, const gpu::RenderingInfo& info) = 0;
    };
}

#endif //VRAKTAL_RHI_CORE_GPU_RENDERER_H