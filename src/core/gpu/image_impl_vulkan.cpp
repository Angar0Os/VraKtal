#include "../src/core/gpu/image_impl_vulkan.h"
#include "../src/core/gpu/gpuDevice_impl_glfw_vulkan.h"

using namespace rhi::vulkan;

ImageVulkan::ImageVulkan(GpuDeviceVulkan& _device, VkImage _image, VkImageView _view, VkFormat _format, uint32_t _width, uint32_t _height)
    : m_device(_device), m_image(_image), m_view(_view), m_format(_format), m_width(_width), m_height(_height)
{
    
}

ImageVulkan::~ImageVulkan()
{
}
