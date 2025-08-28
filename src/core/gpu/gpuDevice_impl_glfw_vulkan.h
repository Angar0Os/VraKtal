#ifndef VRAKTAL_CORE_GPU_GPUDEVICE_H
#define VRAKTAL_CORE_GPU_GPUDEVICE_H
#pragma once

#include "../src/vkb/VkBootstrap.h"
#include <core/gpu/gpuDevice.h>
#include <vma/vk_mem_alloc.h>
#include <vector>

namespace rhi::vulkan
{
    class WindowVulkan;
    class CommandBufferVulkan;

    class GpuDeviceVulkan final : public core::gpu::GpuDevice
    {
    public:
        explicit GpuDeviceVulkan(const WindowVulkan& _window);
        ~GpuDeviceVulkan() override;

        void WaitIdle();

        core::gpu::CommandBuffer* CreateCommandBuffer() override;
        void DestroyCommandBuffer(core::gpu::CommandBuffer* _commandBuffer) override;

        void RecreateSwapchain() override;

        VkDevice Device() const { return m_device; }
        VkPhysicalDevice PhysicalDevice() const { return m_physicalDevice; }
        VkQueue GraphicsQueue() const { return m_graphicsQueue; }
        uint32_t GraphicsQueueFamily() const { return m_graphicsQueueFamily; }
        VmaAllocator Allocator() const { return m_allocator; }

    private:
        void CreateInstance();
        void CreateSurface(const WindowVulkan& window);
        void PickPhysicalDevice();
        void CreateLogicalDevice();
        void CreateAllocator();
        void CreateSwapchain(uint32_t width, uint32_t height);
        void DestroySwapchain();
        void CreateCommandPool();
        void DestroyCommandPool();

        vkb::Instance m_instance;
        VkSurfaceKHR m_surface = VK_NULL_HANDLE;
        vkb::PhysicalDevice m_physicalDevice;
        VkDevice m_device = VK_NULL_HANDLE;

        VkQueue m_graphicsQueue = VK_NULL_HANDLE;
        uint32_t m_graphicsQueueFamily = 0;

        VmaAllocator m_allocator = VK_NULL_HANDLE;

        VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
        VkFormat m_swapFormat = VK_FORMAT_B8G8R8A8_UNORM;
        VkExtent2D m_swapExtent = {};
        std::vector<VkImage> m_swapImages;
        std::vector<VkImageView> m_swapImageViews;

        VkCommandPool m_cmdPool = VK_NULL_HANDLE;
    };
}

#endif //VRAKTAL_CORE_GPU_GPUDEVICE_H