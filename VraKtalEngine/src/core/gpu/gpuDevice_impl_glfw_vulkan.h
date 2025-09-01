#ifndef VRAKTAL_CORE_GPU_GPUDEVICE_H
#define VRAKTAL_CORE_GPU_GPUDEVICE_H
#pragma once

#include "../src/vkb/VkBootstrap.h"
#include <core/gpu/gpuDevice.h>
#include <vma/vk_mem_alloc.h>
#include <vector>

namespace rhi::core::gpu { class Image; }

namespace rhi::vulkan
{
    class WindowVulkan;
    class CommandBufferVulkan;
    class ImageVulkan;

    class GpuDeviceVulkan final : public core::gpu::GpuDevice
    {
    public:
        explicit GpuDeviceVulkan(const WindowVulkan& _window);
        ~GpuDeviceVulkan() override;

        void WaitIdle();

        core::gpu::CommandBuffer* CreateCommandBuffer() override;
        void DestroyCommandBuffer(core::gpu::CommandBuffer* _commandBuffer) override;

        void RecreateSwapchain() override;

        bool BeginFrame(uint32_t& imageIndex);
        void EndFrame(uint32_t imageIndex, VkCommandBuffer cmd);

        void WrapSwapchainImages();

        VkDevice Device() const { return m_device; }
        VkPhysicalDevice PhysicalDevice() const { return m_physicalDevice; }
        VkQueue GraphicsQueue() const { return m_graphicsQueue; }
        uint32_t GraphicsQueueFamily() const { return m_graphicsQueueFamily; }
        VmaAllocator Allocator() const { return m_allocator; }

        VkFormat SwapFormat() const { return m_swapFormat; }
        VkExtent2D SwapExtent() const { return m_swapExtent; }

        uint32_t CurrentFrame() const { return m_currentFrame; }

        core::gpu::Image* GetSwapchainImage(uint32_t index) const;
        void UploadToBuffer(VkBuffer dst, const void* data, VkDeviceSize size);
        
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
        void CreateSyncObjects();
        void DestroySyncObjects();
        void DeleteWrappedImages();

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

        std::vector<ImageVulkan*> m_swapchainImageWrappers;

        VkCommandPool m_cmdPool = VK_NULL_HANDLE;

        struct FrameSync {
            VkSemaphore imageAvailable = VK_NULL_HANDLE;
            VkSemaphore renderFinished = VK_NULL_HANDLE;
            VkFence inFlight = VK_NULL_HANDLE;
        };
        static constexpr int OVERLAPPED_FRAMES = 2;
        std::vector<FrameSync> m_frames;
        uint32_t m_currentFrame = 0;
        bool m_framebufferResized = false;
    };
}

#endif //VRAKTAL_CORE_GPU_GPUDEVICE_H