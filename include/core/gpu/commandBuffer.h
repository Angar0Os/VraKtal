#ifndef VRAKTAL_RHI_CORE_GPU_COMMAND_BUFFER_H
#define VRAKTAL_RHI_CORE_GPU_COMMAND_BUFFER_H
#pragma once

#include <cstdint>
#include <core/gpu/renderingInfo.h>

namespace rhi::core::gpu
{
    struct RenderingInfo;
    class Pipeline;
    
    class CommandBuffer {
    public:
        virtual ~CommandBuffer() = default;

        virtual void Begin() = 0;
        virtual void End() = 0;
        virtual void Reset() = 0;

        virtual void BeginRendering(const RenderingInfo& info) = 0;
        virtual void EndRendering() = 0;

        virtual void BindPipeline(Pipeline* pipeline) = 0;
        virtual void Draw(uint32_t vertexCount) = 0;
    };
}

#endif //VRAKTAL_RHI_CORE_COMMAND_BUFFER_H
