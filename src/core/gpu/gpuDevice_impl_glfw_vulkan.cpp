#include "../src/core/gpu/gpuDevice_impl_glfw_vulkan.h"
#include "../src/core/gpu/window_impl_vulkan.h"
#include "../src/core/gpu/commandBuffer_impl_vulkan.h"
#include "../src/vkb/VkBootstrap.h"

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include <stdexcept>

using namespace rhi::vulkan;

GpuDeviceVulkan::GpuDeviceVulkan(const WindowVulkan& _window)
{
    CreateInstance();
    CreateSurface(_window);
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateAllocator();
    CreateSwapchain(_window.Size().first, _window.Size().second);
    CreateCommandPool();
}

GpuDeviceVulkan::~GpuDeviceVulkan()
{
    WaitIdle();
    DestroyCommandPool();
    DestroySwapchain();
    if (m_allocator) vmaDestroyAllocator(m_allocator);
    if (m_device) vkDestroyDevice(m_device, nullptr);
    if (m_surface) vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    if (m_instance) vkDestroyInstance(m_instance, nullptr);
}

void GpuDeviceVulkan::WaitIdle()
{
    vkDeviceWaitIdle(m_device);
}

rhi::core::gpu::CommandBuffer* GpuDeviceVulkan::CreateCommandBuffer()
{
    return reinterpret_cast<rhi::core::gpu::CommandBuffer*>(new CommandBufferVulkan(*this));
}

void GpuDeviceVulkan::DestroyCommandBuffer(rhi::core::gpu::CommandBuffer* commandBuffer)
{
    delete reinterpret_cast<CommandBufferVulkan*>(commandBuffer);
}

void GpuDeviceVulkan::RecreateSwapchain()
{
    WaitIdle();
    DestroySwapchain();
    CreateSwapchain(m_swapExtent.width, m_swapExtent.height);
}

void GpuDeviceVulkan::CreateInstance()
{
    vkb::InstanceBuilder builder;
    auto instance = builder
        .set_app_name("Vulkan Window")
        .request_validation_layers(true)
        .use_default_debug_messenger()
        .require_api_version(1, 3, 0)
        .build();

    if (!instance)
    {
        throw std::runtime_error("failed to create instance");
    }

    m_instance = instance.value();
}

void GpuDeviceVulkan::CreateSurface(const WindowVulkan& _window)
{
    if (glfwCreateWindowSurface(m_instance, _window.GlfwHandle(), nullptr, &m_surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface");
    }
}

void GpuDeviceVulkan::PickPhysicalDevice()
{
    VkPhysicalDeviceVulkan13Features f13 { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
    f13.dynamicRendering = VK_TRUE;
    f13.synchronization2 = VK_TRUE;

    vkb::PhysicalDeviceSelector selector { m_instance, m_surface };
    auto physical_device = selector
    .set_surface(m_surface)
        .set_minimum_version(1,3)
        .set_required_features_13(f13)
        .prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
        .select();

    if (!physical_device)
    {
        throw std::runtime_error("Physical device selection failed");
    }

    m_physicalDevice = physical_device.value();
}

void GpuDeviceVulkan::CreateLogicalDevice()
{
    vkb::DeviceBuilder builder{ m_physicalDevice };
    auto device = builder.build();

    if (!device)
    {
        throw std::runtime_error("Logical device creation failed");
    }
    
    m_device = device.value();
    m_graphicsQueue = device.value().get_queue(vkb::QueueType::graphics).value();
    m_graphicsQueueFamily = device.value().get_queue_index(vkb::QueueType::graphics).value();
}

void GpuDeviceVulkan::CreateAllocator()
{
    VmaAllocatorCreateInfo alloc_info{};
    alloc_info.instance = m_instance;
    alloc_info.physicalDevice = m_physicalDevice;
    alloc_info.device = m_device;
    alloc_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

    if (vmaCreateAllocator(&alloc_info, &m_allocator) != VK_SUCCESS)
    {
        throw std::runtime_error("VMA allocator creation failed");
    }
}

void GpuDeviceVulkan::CreateSwapchain(uint32_t _width, uint32_t _height)
{
    vkb::SwapchainBuilder swapchainBuilder { m_physicalDevice, m_device, m_surface };
    auto swapchain = swapchainBuilder
        .set_desired_extent(_width, _height)
        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
        .set_desired_format({ m_swapFormat, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
        .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        .build();

    if (!swapchain)
    {
        throw std::runtime_error("Swapchain creation failed");
    }
    
    auto value = swapchain.value();
    m_swapchain = value.swapchain;
    m_swapExtent = value.extent;
    m_swapImages = value.get_images().value();
    m_swapImageViews = value.get_image_views().value();
}

void GpuDeviceVulkan::DestroySwapchain()
{
    for (auto v : m_swapImageViews)
    {
        vkDestroyImageView(m_device, v, nullptr);
    }

    m_swapImageViews.clear();
    m_swapImages.clear();
    
    if (m_swapchain)
        {
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
        m_swapchain = VK_NULL_HANDLE;
    }
}

void GpuDeviceVulkan::CreateCommandPool()
{
    VkCommandPoolCreateInfo commandPool_info{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    commandPool_info.queueFamilyIndex = m_graphicsQueueFamily;
    commandPool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    
    if (vkCreateCommandPool(m_device, &commandPool_info, nullptr, &m_cmdPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Command pool creation failed");
    }
}

void GpuDeviceVulkan::DestroyCommandPool()
{
    if (m_cmdPool)
        {
        vkDestroyCommandPool(m_device, m_cmdPool, nullptr);
        m_cmdPool = VK_NULL_HANDLE;
    }
}