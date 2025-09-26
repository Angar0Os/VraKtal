#ifndef VRAKTAL_CORE_RHI_RENDERER_H
#define VRAKTAL_CORE_RHI_RENDERER_H
#pragma once

#include <core/rhi/commandBuffer.h>
#include <core/rhi/renderingInfo.h>

namespace core::rhi
{
    class Renderer
    {
    public:
        virtual ~Renderer() = default;
        virtual void Render(CommandBuffer& cmd, const RenderingInfo& info, uint32_t imageIndex) = 0;
    };
}

#endif //VRAKTAL_CORE_RHI_RENDERER_H