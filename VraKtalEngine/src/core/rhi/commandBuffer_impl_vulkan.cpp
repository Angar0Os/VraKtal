#include "../src/core/rhi/commandBuffer_impl_vulkan.h"
#include "../src/core/rhi/gpuDevice_impl_glfw_vulkan.h"
#include "../src/core/rhi/pipeline_impl_vulkan.h"
#include "../src/core/rhi/image_impl_vulkan.h"

#include <stdexcept>
#include <vector>

using namespace core::rhi::vulkan;

CommandBufferVulkan::CommandBufferVulkan(GpuDeviceVulkan& _device)
    : m_device(_device)
{
    VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    poolInfo.queueFamilyIndex = m_device.GraphicsQueueFamily();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(m_device.Device(), &poolInfo, nullptr, &m_pool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create command pool");

    VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocInfo.commandPool = m_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(m_device.Device(), &allocInfo, &m_commandBuffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate command buffer");
}

CommandBufferVulkan::~CommandBufferVulkan()
{
    if (m_commandBuffer != VK_NULL_HANDLE)
    {
        vkFreeCommandBuffers(m_device.Device(), m_pool, 1, &m_commandBuffer);
    }
    if (m_pool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(m_device.Device(), m_pool, nullptr);
    }
}

void CommandBufferVulkan::Begin()
{
    vkResetCommandBuffer(m_commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(m_commandBuffer, &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("Failed to begin recording command buffer");
}

void CommandBufferVulkan::End()
{
    if (vkEndCommandBuffer(m_commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("vkEndCommandBuffer failed");
    }
}

void CommandBufferVulkan::BeginRendering(const RenderingInfo& info, uint32_t imageIndex)
{
    std::vector<VkRenderingAttachmentInfo> attachments;
    auto* swapImg = static_cast<ImageVulkan*>(m_device.GetSwapchainImage(imageIndex));

    for (auto& att : info.colorAttachments)
    {
        VkRenderingAttachmentInfo vkAttachmentInfo{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        vkAttachmentInfo.imageView = swapImg->View();
        vkAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        vkAttachmentInfo.loadOp = (att.loadOp == LoadOp::Clear) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        vkAttachmentInfo.storeOp = (att.storeOp == StoreOp::Store) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
        vkAttachmentInfo.clearValue.color = { att.clearValue.r, att.clearValue.g, att.clearValue.b, att.clearValue.a };
        attachments.push_back(vkAttachmentInfo);
    }

    VkRenderingAttachmentInfo depthAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
    depthAttachment.imageView = m_device.DepthImageView();
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.clearValue.depthStencil = { 1.0f, 0 };

    VkRenderingInfo vkInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
    vkInfo.renderArea.extent.width = info.width;
    vkInfo.renderArea.extent.height = info.height;
    vkInfo.layerCount = 1;
    vkInfo.colorAttachmentCount = static_cast<uint32_t>(attachments.size());
    vkInfo.pColorAttachments = attachments.data();
    vkInfo.pDepthAttachment = &depthAttachment;

    TransitionImageLayout(
        swapImg->GetNative(),
        swapImg->Format(),
        swapImg->CurrentLayout(),
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    );

    vkCmdBeginRendering(m_commandBuffer, &vkInfo);
}

void CommandBufferVulkan::EndRendering(uint32_t imageIndex)
{
    auto* swapImg = static_cast<ImageVulkan*>(m_device.GetSwapchainImage(imageIndex));

    vkCmdEndRendering(m_commandBuffer);

    TransitionImageLayout(
        swapImg->GetNative(),
        swapImg->Format(),
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    );

    swapImg->SetLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
}

void CommandBufferVulkan::BindPipeline(Pipeline* pipeline)
{
    auto* vkPipeline = reinterpret_cast<PipelineVulkan*>(pipeline);
    vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline->GetNative());
}

void CommandBufferVulkan::Draw(uint32_t vertexCount, uint32_t width, uint32_t height)
{
    VkViewport viewport{};
    viewport.x = 0.f;
    viewport.y = 0.f;
    viewport.width = static_cast<float>(width);
    viewport.height = static_cast<float>(height);
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;
    vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = { width, height };
    vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);

    vkCmdDraw(m_commandBuffer, vertexCount, 1, 0, 0);
}

void CommandBufferVulkan::TransitionImageLayout(VkImage image, VkFormat, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR &&
        newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
    {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = 0;
        srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }

    vkCmdPipelineBarrier(
        m_commandBuffer,
        srcStage, dstStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
}