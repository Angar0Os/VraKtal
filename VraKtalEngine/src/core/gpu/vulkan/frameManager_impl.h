#ifndef VRAKTAL_CORE_GPU_VULKAN_FRAMEMANAGER_H
#define VRAKTAL_CORE_GPU_VULKAN_FRAMEMANAGER_H
#pragma once

#include <core/gpu/frameManager.h>

#include <vulkan/vulkan.h>
#include <vector>

namespace core::gpu { class Device; }

namespace core::gpu
{
    struct FrameManager::Impl
    {
        explicit Impl(Device& device);
        ~Impl();

        void BeginFrame(uint32_t frameIndex);
        uint32_t AcquireNextImage(uint32_t frameIndex);
        void* GetImageAvailableSemaphore(uint32_t frameIndex) const;
        void* GetRenderFinishedSemaphore(uint32_t imageIndex) const;
        void* GetInFlightFence(uint32_t frameIndex) const;
        void SubmitDefaultTransitionIfNeeded(uint32_t frameIndex, uint32_t imageIndex);
        void Present(uint32_t imageIndex);
        void Cleanup();

    private:
        Device& device;

        VkDevice vkDevice = VK_NULL_HANDLE;
        VkQueue  vkQueue = VK_NULL_HANDLE;
        VkCommandPool vkCommandPool = VK_NULL_HANDLE;

        std::vector<VkSemaphore> imageAvailable;
        std::vector<VkSemaphore> renderFinished;
        std::vector<VkFence>     inFlightFences;
        std::vector<VkFence>     imagesInFlight;
        std::vector<VkCommandBuffer> tempCmdBufs;

        uint32_t swapchainImageCount = 0;
    };
}

#endif //VRAKTAL_CORE_GPU_VULKAN_FRAMEMANAGER_H