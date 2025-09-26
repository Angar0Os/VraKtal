#ifndef VRAKTAL_CORE_RHI_GPUDEVICE_H
#define VRAKTAL_CORE_RHI_GPUDEVICE_H
#pragma once

namespace core::rhi
{
    class Window;
    class CommandBuffer;
    
    class GpuDevice
    {
    public:
        virtual ~GpuDevice() = default;

        virtual CommandBuffer* CreateCommandBuffer() = 0;
        virtual void DestroyCommandBuffer(CommandBuffer* _commandBuffer) = 0;

        virtual void RecreateSwapchain() = 0;
    private:
    };
}

#endif //VRAKTAL_CORE_RHI_GPUDEVICE_H