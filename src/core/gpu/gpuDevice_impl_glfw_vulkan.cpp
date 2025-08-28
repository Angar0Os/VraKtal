#include "../src/core/gpu/gpuDevice_impl_glfw_vulkan.h"
#include "../src/core/gpu/window_impl_vulkan.h"
#include "../src/core/gpu/commandBuffer_impl_vulkan.h"
#include "../src/core/gpu/image_impl_vulkan.h"
#include "../src/vkb/VkBootstrap.h"

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include <stdexcept>
#include <array>
#include <limits>

using namespace rhi::vulkan;
using namespace rhi::core::gpu;

GpuDeviceVulkan::GpuDeviceVulkan(const WindowVulkan& _window)
{
    CreateInstance();
    CreateSurface(_window);
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateAllocator();
    CreateSwapchain(_window.Size().first, _window.Size().second);
    CreateCommandPool();
    CreateSyncObjects();
}

GpuDeviceVulkan::~GpuDeviceVulkan()
{
    WaitIdle();
    DestroySyncObjects();
    DeleteWrappedImages();
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
    return reinterpret_cast<core::gpu::CommandBuffer*>(new CommandBufferVulkan(*this));
}

void GpuDeviceVulkan::DestroyCommandBuffer(core::gpu::CommandBuffer* commandBuffer)
{
    delete reinterpret_cast<CommandBufferVulkan*>(commandBuffer);
}

void GpuDeviceVulkan::RecreateSwapchain()
{
    WaitIdle();
    DeleteWrappedImages();
    DestroySwapchain();
    CreateSwapchain(m_swapExtent.width, m_swapExtent.height);
    WrapSwapchainImages();
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
    VkPhysicalDeviceVulkan13Features f13 { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
    f13.dynamicRendering = VK_TRUE;
    f13.synchronization2 = VK_TRUE;

    vkb::PhysicalDeviceSelector selector { m_instance, m_surface };
    auto physical_device = selector
        .set_surface(m_surface)
        .set_minimum_version(1,3)
        .set_required_features_13(f13)
        .prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
        .allow_any_gpu_device_type(false)
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
        .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
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
    m_swapFormat = value.image_format;
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

void GpuDeviceVulkan::CreateSyncObjects()
{
    m_frames.resize(OVERLAPPED_FRAMES);
    VkSemaphoreCreateInfo semInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < OVERLAPPED_FRAMES; ++i)
    {
        if (vkCreateSemaphore(m_device, &semInfo, nullptr, &m_frames[i].imageAvailable) != VK_SUCCESS ||
            vkCreateSemaphore(m_device, &semInfo, nullptr, &m_frames[i].renderFinished) != VK_SUCCESS ||
            vkCreateFence(m_device, &fenceInfo, nullptr, &m_frames[i].inFlight) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create sync objects");
        }
    }
}

void GpuDeviceVulkan::DestroySyncObjects()
{
    for (auto& f : m_frames)
    {
        if (f.imageAvailable) vkDestroySemaphore(m_device, f.imageAvailable, nullptr);
        if (f.renderFinished) vkDestroySemaphore(m_device, f.renderFinished, nullptr);
        if (f.inFlight) vkDestroyFence(m_device, f.inFlight, nullptr);
    }
    m_frames.clear();
}

void GpuDeviceVulkan::WrapSwapchainImages()
{
    DeleteWrappedImages();
    m_swapchainImageWrappers.reserve(m_swapImages.size());
    for (size_t i = 0; i < m_swapImages.size(); ++i)
    {
        auto* wrapper = new ImageVulkan(*this, m_swapImages[i], m_swapImageViews[i], m_swapExtent.width, m_swapExtent.height);
        m_swapchainImageWrappers.push_back(wrapper);
    }
}

void GpuDeviceVulkan::DeleteWrappedImages()
{
    for (auto* img : m_swapchainImageWrappers)
    {
        delete img;
    }
    m_swapchainImageWrappers.clear();
}

bool GpuDeviceVulkan::BeginFrame(uint32_t& imageIndex)
{
    FrameSync& sync = m_frames[m_currentFrame];

    vkWaitForFences(m_device, 1, &sync.inFlight, VK_TRUE, UINT64_MAX);

    VkResult res = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, sync.imageAvailable, VK_NULL_HANDLE, &imageIndex);

    if (res == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapchain();
        return false;
    }
    else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire swapchain image");
    }

    vkResetFences(m_device, 1, &sync.inFlight);
    return true;
}

void GpuDeviceVulkan::EndFrame(uint32_t imageIndex, VkCommandBuffer cmd)
{
    FrameSync& sync = m_frames[m_currentFrame];

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &sync.imageAvailable;
    submit.pWaitDstStageMask = &waitStage;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &sync.renderFinished;

    if (vkQueueSubmit(m_graphicsQueue, 1, &submit, sync.inFlight) != VK_SUCCESS)
        throw std::runtime_error("Queue submit failed");

    VkPresentInfoKHR present{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &sync.renderFinished;
    present.swapchainCount = 1;
    present.pSwapchains = &m_swapchain;
    present.pImageIndices = &imageIndex;

    VkResult res = vkQueuePresentKHR(m_graphicsQueue, &present);

    if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
    {
        RecreateSwapchain();
    }
    else if (res != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present swapchain image");
    }

    m_currentFrame = (m_currentFrame + 1) % OVERLAPPED_FRAMES;
}

rhi::core::gpu::Image* GpuDeviceVulkan::GetSwapchainImage(uint32_t index) const
{
    return m_swapchainImageWrappers[index];
}