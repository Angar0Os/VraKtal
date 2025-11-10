#ifndef VRAKTAL_CORE_GPU_FRAMEMANAGER_H
#define VRAKTAL_CORE_GPU_FRAMEMANAGER_H
#pragma once

#include <cstdint>
#include <cstddef>

namespace core
{
    class Window;
    namespace gpu
    {
        class Device;

        class FrameManager
        {
        public:
            explicit FrameManager(Device& device);
            ~FrameManager();

            void BeginFrame(uint32_t frameIndex);

            uint32_t AcquireNextImage(uint32_t frameIndex);

            void* GetImageAvailableSemaphore(uint32_t frameIndex) const;
            void* GetRenderFinishedSemaphore(uint32_t imageIndex) const;
            void* GetInFlightFence(uint32_t frameIndex) const;

            void SubmitDefaultTransitionIfNeeded(uint32_t frameIndex, uint32_t imageIndex);

            void Present(uint32_t imageIndex);

            void Cleanup();

            static constexpr uint32_t FRAMES_IN_FLIGHT = 2;

        private:
            struct Impl;
            Impl* m_impl = nullptr;
        };
    }
}

#endif //VRAKTAL_CORE_GPU_FRAMEMANAGER_H