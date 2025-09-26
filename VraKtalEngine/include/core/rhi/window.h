#ifndef VRAKTAL_CORE_RHI_WINDOW_H
#define VRAKTAL_CORE_RHI_WINDOW_H
#pragma once

#include <cstdint>
#include <utility>

namespace core::rhi
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

#endif //VRAKTAL_CORE_RHI_WINDOW_H
