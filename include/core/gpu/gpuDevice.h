#ifndef VRAKTAL_RHI_CORE_GPU_GPUDEVICE_H
#define VRAKTAL_RHI_CORE_GPU_GPUDEVICE_H
#pragma once

#include <glm/glm.hpp>
#include <memory>

namespace rhi::core::gpu
{
    struct WindowDescriptor
    {
        const char* windowTitle = "VraKtalEngine";
        glm::uvec2 windowSize = { 1280, 720 };
        bool resizable = false;
    };

    class GpuDevice
    {
    private:
        struct Internal;
        std::unique_ptr<Internal> m_Internal;
    public:
        GpuDevice() = default;
        GpuDevice(const WindowDescriptor& windowDesc);
        
        ~GpuDevice() noexcept;

        Internal& GetInternal() { return *m_Internal;}
    };
}

#endif //VRAKTAL_RHI_CORE_GPU_GPUDEVICE_H