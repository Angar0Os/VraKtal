#include "gpuDevice_impl_glfw_vulkan.h"
#include "../gpu-details/vkInitializers.h"

#include <stdexcept>
#include <GLFW/glfw3.h>

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "vulkan-1.lib")

using namespace rhi::core::gpu;
using namespace core::gpu_details;

GpuDevice::Internal::Internal(GpuDevice* parent)
    :m_parent(parent)
{
    
}

GpuDevice::~GpuDevice()
{
    
}


GpuDevice::GpuDevice(const WindowDescriptor& windowDesc)
    : m_Internal(std::make_unique<Internal>(this))
{
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, windowDesc.resizable ? GLFW_TRUE : GLFW_FALSE);
    m_Internal->window = glfwCreateWindow(windowDesc.windowSize.x, windowDesc.windowSize.y, windowDesc.windowTitle, nullptr, nullptr);

    if (!m_Internal->window)
    {
        throw std::runtime_error("Failed to create GLFW window");
    }

    m_Internal->windowExtent = { windowDesc.windowSize.x, windowDesc.windowSize.y };

    vkb::InstanceBuilder instanceBuilder;

    auto instanceResult = instanceBuilder.set_app_name("VraKtal")
        .request_validation_layers(true)
        .use_default_debug_messenger()
        .require_api_version(1, 3, 0)
        .build();   

    if (!instanceResult)
    {
        throw std::runtime_error("Failed to create Vulkan instance" + instanceResult.error().message());
    }

    vkb::Instance instance = instanceResult.value();
    m_Internal->instance = instance;

    VkResult result = glfwCreateWindowSurface(m_Internal->instance.instance, m_Internal->window, nullptr, &m_Internal->surface);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Vulkan surface" + std::to_string(result));
    }
    
    VkPhysicalDeviceVulkan13Features deviceFeatures { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
    deviceFeatures.dynamicRendering = true;
    deviceFeatures.synchronization2 = true;
    
    vkb::PhysicalDeviceSelector selector { m_Internal->instance };
    auto physicalDeviceResult = selector
        .set_minimum_version(1, 3)
        .set_required_features_13(deviceFeatures)
        .set_surface(m_Internal->surface)
        .prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
        .allow_any_gpu_device_type(false)
        .select();

    if (!physicalDeviceResult)
    {
        throw std::runtime_error("Failed to create Vulkan physical device" + physicalDeviceResult.error().message());
    }

    vkb::PhysicalDevice physicalDevice = physicalDeviceResult.value();
    
    vkb::DeviceBuilder deviceBuilder { physicalDevice };
    auto vkbDeviceResult = deviceBuilder.build();

    if (!vkbDeviceResult)
    {
        throw std::runtime_error("Failed to create logical device" + vkbDeviceResult.error().message());
    }

    vkb::Device vkbDevice = vkbDeviceResult.value();
    m_Internal->device = vkbDevice.device;
    m_Internal->physicalDevice = physicalDevice.physical_device;

    
    m_Internal->graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    m_Internal->graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = m_Internal->physicalDevice;
    allocatorInfo.device = m_Internal->device;
    allocatorInfo.instance = m_Internal->instance;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    vmaCreateAllocator(&allocatorInfo, &m_Internal->allocator);

    m_Internal->CreateSwapchain(m_Internal->windowExtent.width, m_Internal->windowExtent.height);

    VkExtent3D drawImageExtent = {
        m_Internal->windowExtent.width,
        m_Internal->windowExtent.height,
        1
    };

    m_Internal->drawImage.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
    m_Internal->drawImage.imageExtent = drawImageExtent;

    VkImageUsageFlags drawImageUsages{};
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageCreateInfo rimg_info = ImageCreateInfo(m_Internal->drawImage.imageFormat, drawImageUsages, drawImageExtent);

    VmaAllocationCreateInfo rimg_allocinfo = {};
    rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    auto error = vmaCreateImage(m_Internal->allocator, &rimg_info, &rimg_allocinfo, &m_Internal->drawImage.image, &m_Internal->drawImage.allocation, nullptr);

    VkImageViewCreateInfo rview_info = ImageViewCreateInfo(m_Internal->drawImage.imageFormat, m_Internal->drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

    vkCreateImageView(m_Internal->device, &rview_info, nullptr, &m_Internal->drawImage.imageView);

    m_Internal->depthImage.imageFormat = VK_FORMAT_D32_SFLOAT;
    m_Internal->depthImage.imageExtent = drawImageExtent;

    VkImageUsageFlags depthImageUsages{};
    depthImageUsages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkImageCreateInfo dimg_info = ImageCreateInfo(m_Internal->depthImage.imageFormat, depthImageUsages, drawImageExtent);

    vmaCreateImage(m_Internal->allocator, &dimg_info, &rimg_allocinfo, &m_Internal->depthImage.image, &m_Internal->depthImage.allocation, nullptr);
    VkImageViewCreateInfo dview_info = ImageViewCreateInfo(m_Internal->depthImage.imageFormat, m_Internal->depthImage.image, VK_IMAGE_ASPECT_DEPTH_BIT);

    vkCreateImageView(m_Internal->device, &dview_info, nullptr, &m_Internal->depthImage.imageView);
} 

void GpuDevice::Internal::CreateSwapchain(uint32_t width, uint32_t height)
{
    vkb::SwapchainBuilder swapchainBuilder { physicalDevice, device, surface };

    swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

    vkb::Swapchain vkbSwapchain = swapchainBuilder
        .set_desired_format(VkSurfaceFormatKHR{ .format = swapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
        .set_desired_extent(width, height)
        .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        .build()
        .value();

    swapchainExtent = vkbSwapchain.extent;
    swapchain = vkbSwapchain.swapchain;
    swapchainImages = vkbSwapchain.get_images().value();
    swapchainImageViews = vkbSwapchain.get_image_views().value();
}

void GpuDevice::Internal::DestroySwapchain()
{
    vkDestroySwapchainKHR(device, swapchain, nullptr);

    for (int i = 0; i < swapchainImageViews.size(); ++i)
    {
        vkDestroyImageView(device, swapchainImageViews[i], nullptr);
    }
}