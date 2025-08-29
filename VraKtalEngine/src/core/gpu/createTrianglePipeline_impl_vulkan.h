#ifndef VRAKTAL_RHI_CORE_GPU_CREATE_TRIANGLE_PIPELINE_VULKAN_H
#define VRAKTAL_RHI_CORE_GPU_CREATE_TRIANGLE_PIPELINE_VULKAN_H
#pragma once

#include <core/gpu/pipeline.h>

namespace rhi::vulkan
{
    class GpuDeviceVulkan;
    core::gpu::Pipeline* CreateTrianglePipeline(GpuDeviceVulkan& device);
}

#endif // VRAKTAL_RHI_CORE_GPU_CREATE_TRIANGLE_PIPELINE_VULKAN_H