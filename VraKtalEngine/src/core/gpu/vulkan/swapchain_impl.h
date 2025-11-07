#ifndef VRAKTAL_CORE_GPU_VULKAN_SWAPCHAIN_IMPL_H
#define VRAKTAL_CORE_GPU_VULKAN_SWAPCHAIN_IMPL_H
#pragma once

#include <core/gpu/swapchain.h>
#include <vulkan/vulkan_raii.hpp>

namespace core::gpu
{
    struct Swapchain::Impl
    {
    private:
        Swapchain& parent;
        vk::raii::Device& device;
        vk::raii::PhysicalDevice& physicalDevice;

        vk::raii::SwapchainKHR swapchain;
        std::vector<vk::Image> images;
        std::vector<vk::raii::ImageView> imageViews;

        vk::Format format;
        vk::Extent2D extent;

        vk::SurfaceFormatKHR ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats,
            TextureFormat preferredFormat);
        vk::PresentModeKHR ChoosePresentMode(const std::vector<vk::PresentModeKHR>& availableModes,
            PresentMode preferredMode);
        vk::Extent2D ChooseExtent(const vk::SurfaceCapabilitiesKHR& capabilities,
            uint32_t width, uint32_t height);

    public:
        explicit Impl(Swapchain& p, vk::raii::Device& dev, vk::raii::PhysicalDevice& physDev,
            const SwapchainCreateInfo& info);
        ~Impl();

        vk::raii::SwapchainKHR& GetSwapchain();
        const vk::raii::SwapchainKHR& GetSwapchain() const;

        uint32_t GetImageCount() const;
        SwapchainImage GetImage(uint32_t index) const;

        TextureFormat GetFormat() const;
        uint32_t GetWidth() const;
        uint32_t GetHeight() const;

        uint32_t AcquireNextImage(vk::Semaphore semaphore, uint64_t timeout);
    };
}

#endif // VRAKTAL_CORE_GPU_VULKAN_SWAPCHAIN_IMPL_H