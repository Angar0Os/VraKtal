#ifndef VRAKTAL_CORE_RHI_COMMAND_BUFFER_H
#define VRAKTAL_CORE_RHI_COMMAND_BUFFER_H
#pragma once

#include <cstdint>

#include <core/rhi/renderingInfo.h>

namespace core::rhi
{
    struct RenderingInfo;
    class Pipeline;
    
    class CommandBuffer 
    {
    public:
        virtual ~CommandBuffer() = default;

        virtual void Begin() = 0;
        virtual void End() = 0;

        virtual void BeginRendering(const RenderingInfo& info, uint32_t imageIndex) = 0;
        virtual void EndRendering(uint32_t imageIndex) = 0;

        virtual void BindPipeline(Pipeline* pipeline) = 0;
        virtual void Draw(uint32_t vertexCount, uint32_t width, uint32_t height) = 0;
    };
}

#endif //VRAKTAL_CORE_RHI_COMMAND_BUFFER_H
