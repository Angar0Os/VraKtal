#ifndef VRAKTAL_CORE_RHI_PIPELINE_VK_H
#define VRAKTAL_CORE_RHI_PIPELINE_VK_H
#pragma once

#include <vulkan/vulkan.h>
#include <core/rhi/pipeline.h>

namespace core::rhi::vulkan
{
    class GpuDeviceVulkan;

    class PipelineVulkan final : public Pipeline
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

#endif //VRAKTAL_CORE_RHI_PIPELINE_VK_H
