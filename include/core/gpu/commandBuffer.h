#ifndef VRAKTAL_RHI_CORE_GPU_COMMAND_BUFFER_H
#define VRAKTAL_RHI_CORE_GPU_COMMAND_BUFFER_H
#pragma once

namespace rhi::core::gpu
{
    class CommandBuffer {
    public:
        virtual ~CommandBuffer() = default;

        virtual void Begin() = 0;
        virtual void End() = 0;
        virtual void Reset() = 0;
    };
}

#endif //VRAKTAL_RHI_CORE_COMMAND_BUFFER_H
