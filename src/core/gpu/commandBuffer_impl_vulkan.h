#ifndef VRAKTAL_CORE_GPU_COMMAND_BUFFER_H
#define VRAKTAL_CORE_GPU_COMMAND_BUFFER_H
#pragma once

#include <core/gpu/commandBuffer.h>
#include <vulkan/vulkan.h>

namespace rhi::vulkan
{
    class GpuDeviceVulkan;

    class CommandBufferVulkan final : public rhi::core::gpu::CommandBuffer
    {
    public:
        explicit CommandBufferVulkan(GpuDeviceVulkan& _device);
        ~CommandBufferVulkan() override;

        void Begin() override;
        void End() override;
        void Reset() override;

        VkCommandBuffer CommandBuffer() const { return m_commandBuffer; }
    
    private:
        GpuDeviceVulkan& m_device;
        VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
        VkCommandPool m_pool;
    };
}

#endif //VRAKTAL_CORE_GPU_COMMAND_BUFFER_H
