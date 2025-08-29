#ifndef VRAKTAL_RHI_CORE_GPU_IMAGE_VULKAN_H
#define VRAKTAL_RHI_CORE_GPU_IMAGE_VULKAN_H
#pragma once

#include <vulkan/vulkan.h>
#include <core/gpu/image.h>

namespace rhi::vulkan
{
    class GpuDeviceVulkan;

    class ImageVulkan final : public core::gpu::Image
    {
    public:
        ImageVulkan(GpuDeviceVulkan& _device, VkImage _image, VkImageView _view, VkFormat _format ,uint32_t _width, uint32_t _height);
        ~ImageVulkan() override;

        uint32_t Width() const override { return m_width; }
        uint32_t Height() const override { return m_height; }

        VkImageView View() const { return m_view; }
        VkImage GetNative() const { return m_image; } 
        VkFormat Format() const { return m_format; }

        VkImageLayout CurrentLayout() const { return m_currentLayout; }
        void SetLayout(VkImageLayout layout) const { m_currentLayout = layout; }
    private:
        GpuDeviceVulkan& m_device;
        VkImage m_image = VK_NULL_HANDLE;
        VkImageView m_view = VK_NULL_HANDLE;
        VkFormat m_format;
        uint32_t m_width, m_height;

        mutable VkImageLayout m_currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    };
}

#endif //VRAKTAL_RHI_CORE_GPU_IMAGE_VULKAN_H 