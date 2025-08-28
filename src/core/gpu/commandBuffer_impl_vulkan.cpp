#include "../src/core/gpu/commandBuffer_impl_vulkan.h"
#include "../src/core/gpu/gpuDevice_impl_glfw_vulkan.h"
#include "../src/core/gpu/pipeline_impl_vulkan.h"

#include <stdexcept>

using namespace rhi::vulkan;
using namespace rhi::core::gpu;

CommandBufferVulkan::CommandBufferVulkan(GpuDeviceVulkan& _device)
    : m_device(_device)
{
    VkCommandPoolCreateInfo commandPoolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
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

void CommandBufferVulkan::BeginRendering(const RenderingInfo& info)
{
    std::vector<VkRenderingAttachmentInfo> attachments;
    for (auto& att : info.colorAttachments)
    {
        VkRenderingAttachmentInfo vkAttachmentInfo{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        vkAttachmentInfo.imageView = VK_NULL_HANDLE;
        vkAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
        vkAttachmentInfo.loadOp = (att.loadOp == LoadOp::Clear) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        vkAttachmentInfo.storeOp = (att.storeOp == StoreOp::Store) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
        vkAttachmentInfo.clearValue.color = { att.clearValue.r, att.clearValue.g, att.clearValue.b, att.clearValue.a };
        attachments.push_back(vkAttachmentInfo);
    }

    VkRenderingInfo vkInfo { VK_STRUCTURE_TYPE_RENDERING_INFO };
    vkInfo.renderArea.extent.width = info.width;
    vkInfo.renderArea.extent.height = info.height;
    vkInfo.layerCount = 1;
    vkInfo.colorAttachmentCount = static_cast<uint32_t>(attachments.size());
    vkInfo.pColorAttachments = attachments.data();

    vkCmdBeginRendering(m_commandBuffer, &vkInfo);
}

void CommandBufferVulkan::EndRendering()
{
    vkCmdEndRendering(m_commandBuffer);
}

void CommandBufferVulkan::BindPipeline(rhi::core::gpu::Pipeline* pipeline)
{
    auto* vkPipeline = reinterpret_cast<PipelineVulkan*>(pipeline);
    vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline->GetNative());
}

void CommandBufferVulkan::Draw(uint32_t vertexCount)
{
    vkCmdDraw(m_commandBuffer, vertexCount, 1, 0, 0);
}

