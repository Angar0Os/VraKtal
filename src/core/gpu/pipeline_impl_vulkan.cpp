#include "../src/core/gpu/pipeline_impl_vulkan.h"
#include "../src/core/gpu/gpuDevice_impl_glfw_vulkan.h"

using namespace rhi::vulkan;

PipelineVulkan::PipelineVulkan(GpuDeviceVulkan& device, VkPipeline pipeline, VkPipelineLayout layout)
    : m_device(device), m_pipeline(pipeline), m_layout(layout)
{
    
}

PipelineVulkan::~PipelineVulkan()
{
    if (m_pipeline)
    {
        vkDestroyPipeline(m_device.Device(), m_pipeline, nullptr);
    }

    if (m_layout)
    {
        vkDestroyPipelineLayout(m_device.Device(), m_layout, nullptr);
    }
}

