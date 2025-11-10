#ifndef VRAKTAL_GRAPHICS_RENDERER_H
#define VRAKTAL_GRAPHICS_RENDERER_H
#pragma once

#include <functional>
#include <atomic>

#include <core/window.h>
#include <core/gpu/device.h>
#include <core/gpu/frameManager.h>

namespace graphics
{
    using FrameCallback = std::function<bool(uint32_t /*imageIndex*/, void* /*imageAvailableSemaphore*/, void* /*renderFinishedSemaphore*/, void* /*inFlightFence*/)>;

    class Renderer
    {
    public:
        explicit Renderer(core::Window& window, core::gpu::Device& device);
        ~Renderer();

        void SetFrameCallback(FrameCallback cb);
        void DrawFrame();
        void Cleanup();

    private:
        core::Window& m_window;
        core::gpu::Device& m_device;
        core::gpu::FrameManager m_frameManager;
        FrameCallback m_frameCallback;
        std::atomic<bool> m_running;

        uint32_t m_currentFrame;
    };
}

#endif //VRAKTAL_GRAPHICS_RENDERER_H