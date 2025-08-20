#ifndef VRAKTAL_CORE_GPU_GPUDEVICE_H
#define VRAKTAL_CORE_GPU_GPUDEVICE_H
#pragma once

#include "../src/vkb/VkBootstrap.h"

#include <core/gpu/gpuDevice.h>
#include <vma/vk_mem_alloc.h>

#include "../gpu-details/vkInitializers.h"
#include "../gpu-details/vkTypes.h"

struct GLFWwindow;

struct rhi::core::gpu::GpuDevice::Internal
{
private:
    GpuDevice* m_parent;
    
public:
    Internal(GpuDevice* parent);
    ~Internal() = default;

    void CreateSwapchain(uint32_t width, uint32_t height);
    void DestroySwapchain();
    
    GLFWwindow* window;
    VkExtent2D windowExtent;

    vkb::Instance instance;
    VkDevice device;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VmaAllocator allocator;
    VkQueue graphicsQueue;
    uint32_t graphicsQueueFamily;

    VkSwapchainKHR swapchain;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;
    uint32_t swapchainImageCount;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    uint32_t swapchainImageIndex;

    vkTypes::AllocatedImage drawImage;
    vkTypes::AllocatedImage depthImage;
};

#endif //VRAKTAL_CORE_GPU_GPUDEVICE_H