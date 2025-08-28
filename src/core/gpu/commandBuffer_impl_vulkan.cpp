#include "../src/core/gpu/commandBuffer_impl_vulkan.h"
#include "../src/core/gpu/gpuDevice_impl_glfw_vulkan.h"

#include <stdexcept>

using namespace rhi::vulkan;

CommandBufferVulkan::CommandBufferVulkan(GpuDeviceVulkan& _device)
    : m_device(_device)
{
    VkCommandPoolCreateInfo commandPoolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    commandPoolInfo.queueFamilyIndex = _device.GraphicsQueueFamily();
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(_device.Device(), &commandPoolInfo, nullptr, &m_pool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool");    
    }

    VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocInfo.commandPool = m_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(_device.Device(), &allocInfo, &m_commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers");
    }
}

CommandBufferVulkan::~CommandBufferVulkan()
{
    if (m_commandBuffer)
    {
        vkFreeCommandBuffers(m_device.Device(), m_pool, 1, &m_commandBuffer);
    }

    if (m_pool)
    {
        vkDestroyCommandPool(m_device.Device(), m_pool, nullptr);
    }
}

void CommandBufferVulkan::Begin()
{
    VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    if (vkBeginCommandBuffer(m_commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("vkBeginCommandBuffer failed");
    }
}

void CommandBufferVulkan::End()
{
    if (vkEndCommandBuffer(m_commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("vkEndCommandBuffer failed");
    }
}

void CommandBufferVulkan::Reset()
{
    vkResetCommandBuffer(m_commandBuffer, 0);
}
