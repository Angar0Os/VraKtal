#include "gpuDevice_impl_glfw_vulkan.h"

#include <stdexcept>
#include <GLFW/glfw3.h>

#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "vulkan-1.lib")

using namespace rhi::core::gpu;

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

    
} 
