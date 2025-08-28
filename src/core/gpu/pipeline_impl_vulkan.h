#ifndef VRAKTAL_RHI_CORE_GPU_PIPELINE_VULKAN_H
#define VRAKTAL_RHI_CORE_GPU_PIPELINE_VULKAN_H
#pragma once

#include <vulkan/vulkan.h>
#include <core/gpu/pipeline.h>

namespace rhi::vulkan
{
    class GpuDeviceVulkan;

    class PipelineVulkan final : public core::gpu::Pipeline
    {
    public:
        PipelineVulkan(GpuDeviceVulkan& device, VkPipeline pipeline, VkPipelineLayout layout);
        ~PipelineVulkan() override;

        VkPipeline GetNative() const { return m_pipeline; }
        VkPipelineLayout GetLayout() const { return m_layout; }
    private:
        GpuDeviceVulkan& m_device;
        VkPipeline m_pipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_layout = VK_NULL_HANDLE;
    };
}

#endif //VRAKTAL_RHI_CORE_GPU_PIPELINE_VULKAN_H
