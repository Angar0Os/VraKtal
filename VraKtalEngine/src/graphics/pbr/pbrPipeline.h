#ifndef VRAKTAL_SRC_GRAPHICS_PBR_PBR_PIPELINE_H
#define VRAKTAL_SRC_GRAPHICS_PBR_PBR_PIPELINE_H
#pragma once

#include "../src/core/gpu/gpuDevice_impl_glfw_vulkan.h"

#include <vulkan/vulkan.h>
#include <vector>

using namespace rhi::vulkan;

namespace graphics::pbr
{
    class PBRPipeline
    {
    public:
        PBRPipeline(GpuDeviceVulkan& device, VkRenderPass renderPass);
        ~PBRPipeline();

        PipelineVulkan GetPipeline() const { return *m_pipeline; }
        VkPipelineLayout GetPipelineLayout() const { return m_pipelineLayout; }
        VkDescriptorSetLayout GetGlobalSetLayout() const { return m_globalSetLayout; }
        VkDescriptorSetLayout GetMaterialSetLayout() const { return m_materialSetLayout; }

    private:
        GpuDeviceVulkan& m_device;
        PipelineVulkan* m_pipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        VkDescriptorSetLayout m_globalSetLayout = VK_NULL_HANDLE;
        VkDescriptorSetLayout m_materialSetLayout = VK_NULL_HANDLE;
    };
}

#endif // VRAKTAL_SRC_GRAPHICS_PBR_PBR_PIPELINE_H
