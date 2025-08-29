#ifndef VRAKTAL_RHI_CORE_GPU_WINDOW_H
#define VRAKTAL_RHI_CORE_GPU_WINDOW_H
#pragma once

#include <cstdint>
#include <utility>

namespace  rhi::core::gpu
{
    class Window {
    public:
        virtual ~Window() = default;

        virtual void PollEvents() = 0;
        virtual bool ShouldClose() const = 0;

        virtual std::pair<uint32_t,uint32_t> Size() const = 0;
        virtual const char* Title() const = 0;
    };
}

#endif //VRAKTAL_RHI_CORE_GPU_WINDOW_H
