#ifndef VRAKTAL_RHI_CORE_GPU_IMAGE_VULKAN_H
#define VRAKTAL_RHI_CORE_GPU_IMAGE_VULKAN_H
#pragma comment

#include <vulkan/vulkan.h>
#include <core/gpu/image.h>

namespace rhi::vulkan
{
    class GpuDeviceVulkan;

    class ImageVulkan final : public core::gpu::Image
    {
    public:
        ImageVulkan(GpuDeviceVulkan& _device, VkImage _image, VkImageView _view, uint32_t _width, uint32_t _height);
        ~ImageVulkan() override;

        uint32_t Width() const override { return m_width; }
        uint32_t Height() const override { return m_height; }

        VkImageView View() const { return m_view; }
        VkImage GetNative() const { return m_image; } 
    private:
        GpuDeviceVulkan& m_device;
        VkImage m_image = VK_NULL_HANDLE;
        VkImageView m_view = VK_NULL_HANDLE;
        uint32_t m_width, m_height;
    };
}

#endif //VRAKTAL_RHI_CORE_GPU_IMAGE_VULKAN_H 