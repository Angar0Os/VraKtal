#ifndef VRAKTAL_CORE_GPU_GPUDEVICE_H
#define VRAKTAL_CORE_GPU_GPUDEVICE_H
#pragma once

#include "../src/vkb/VkBootstrap.h"

#include <core/gpu/gpuDevice.h>

struct GLFWwindow;

struct rhi::core::gpu::GpuDevice::Internal
{
private:
    GpuDevice* m_parent;
public:
    Internal(GpuDevice* parent);
    ~Internal() = default;
    
    GLFWwindow* window;

    vkb::Instance instance;
    VkDevice device;
    VkSurfaceKHR surface;
};

#endif //VRAKTAL_CORE_GPU_GPUDEVICE_H