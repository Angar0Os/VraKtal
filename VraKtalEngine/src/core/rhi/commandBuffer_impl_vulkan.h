#ifndef VRAKTAL_CORE_RHI_COMMAND_BUFFER_VK_H
#define VRAKTAL_CORE_RHI_COMMAND_BUFFER_VK_H
#pragma once

#include <core/rhi/commandBuffer.h>
#include <vulkan/vulkan.h>

using namespace core::rhi;

namespace core::rhi::vulkan
{
    class GpuDeviceVulkan;

    class CommandBufferVulkan final : public CommandBuffer
    {
    public:
        explicit CommandBufferVulkan(GpuDeviceVulkan& _device);
        ~CommandBufferVulkan() override;

        void Begin() override;
        void End() override;

        void BeginRendering(const RenderingInfo& info, uint32_t imageIndex) override;
        void EndRendering(uint32_t imageIndex) override;
        
        void BindPipeline(Pipeline* pipeline) override;
        void Draw(uint32_t vertexCount, uint32_t width, uint32_t height) override;

        void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

        VkCommandBuffer GetNative() const { return m_commandBuffer; }
    
    private:
        GpuDeviceVulkan& m_device;
        VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
        VkCommandPool m_pool;
    };
}

#endif //VRAKTAL_CORE_RHI_COMMAND_BUFFER_VK_H
