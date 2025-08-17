#include "vkInitializers.h"

VkCommandPoolCreateInfo core::gpu_detail::CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags /*= 0*/)
{
    VkCommandPoolCreateInfo info = { .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    info.queueFamilyIndex = queueFamilyIndex;
    info.flags            = flags;
    return info;
}

VkCommandBufferAllocateInfo core::gpu_detail::CommandBufferAllocateInfo(VkCommandPool pool, bool isSecondary /* = false */, uint32_t count /*= 1*/)
{
    VkCommandBufferAllocateInfo info = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    info.commandPool        = pool;
    info.commandBufferCount = count;
    info.level              = isSecondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    return info;
}

VkCommandBufferBeginInfo core::gpu_detail::CommandBufferBeginInfo(VkCommandBufferUsageFlags flags /*= 0*/)
{
    VkCommandBufferBeginInfo info = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO  };
    info.pInheritanceInfo = nullptr;
    info.flags            = flags;
    return info;
}

VkFenceCreateInfo core::gpu_detail::FenceCreateInfo(VkFenceCreateFlags flags /*= 0*/)
{
    VkFenceCreateInfo info = { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    info.flags = flags;
    return info;
}

VkSemaphoreCreateInfo core::gpu_detail::SemaphoreCreateInfo(VkSemaphoreCreateFlags flags /*= 0*/)
{
    return VkSemaphoreCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .flags = flags
    };
}

VkSemaphoreSubmitInfo core::gpu_detail::SemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore)
{
    VkSemaphoreSubmitInfo submitInfo{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
    submitInfo.semaphore   = semaphore;
    submitInfo.stageMask   = stageMask;
    submitInfo.deviceIndex = 0;
    submitInfo.value       = 1;

    return submitInfo;
}

VkCommandBufferSubmitInfo core::gpu_detail::CommandBufferSubmitInfo(VkCommandBuffer cmd)
{
    VkCommandBufferSubmitInfo info{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
    info.commandBuffer = cmd;
    info.deviceMask    = 0;

    return info;
}

VkSubmitInfo2 core::gpu_detail::SubmitInfo(VkCommandBufferSubmitInfo* cmd, const std::vector<VkSemaphoreSubmitInfo>& signalSemaphoreInfos /*= {}*/, const std::vector<VkSemaphoreSubmitInfo>& waitSemaphoreInfos /*= {}*/)
{
    VkSubmitInfo2 info = { .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
    info.waitSemaphoreInfoCount = uint32_t(waitSemaphoreInfos.size());
    info.pWaitSemaphoreInfos    = info.waitSemaphoreInfoCount ? waitSemaphoreInfos.data() : nullptr;

    info.signalSemaphoreInfoCount = uint32_t(signalSemaphoreInfos.size());
    info.pSignalSemaphoreInfos    = info.signalSemaphoreInfoCount ? signalSemaphoreInfos.data() : nullptr;

    info.commandBufferInfoCount = 1;
    info.pCommandBufferInfos    = cmd;

    return info;
}

VkPresentInfoKHR core::gpu_detail::PresentInfo(const VkSwapchainKHR& swapchain,const VkSemaphore& waitSemaphore, const uint32_t& imageIndex)
{
    VkPresentInfoKHR info = { .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };

    info.pSwapchains = &swapchain;
    info.swapchainCount = 1;

    info.pWaitSemaphores = &waitSemaphore;
    info.waitSemaphoreCount = 1;

    info.pImageIndices = &imageIndex;

    return info;
}

VkRenderingAttachmentInfo core::gpu_detail::AttachmentInfo(VkImageView view, VkClearValue* clear, VkImageLayout layout /*= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL*/)
{
    VkRenderingAttachmentInfo colorAttachment{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
    colorAttachment.imageView   = view;
    colorAttachment.imageLayout = layout;
    colorAttachment.loadOp      = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;

    if (clear)
        colorAttachment.clearValue = *clear;

    return colorAttachment;
}

VkRenderingAttachmentInfo core::gpu_detail::DepthAttachmentInfo( VkImageView view, VkImageLayout layout /*= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL*/)
{
    VkRenderingAttachmentInfo depthAttachment{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
    depthAttachment.imageView                     = view;
    depthAttachment.imageLayout                   = layout;
    depthAttachment.loadOp                        = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp                       = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.clearValue.depthStencil.depth = 0.0f;

    return depthAttachment;
}

VkRenderingInfo core::gpu_detail::RenderingInfo(VkExtent2D renderExtent, const std::vector<VkRenderingAttachmentInfo>& colorAttachments, VkRenderingAttachmentInfo* depthAttachment)
{
    VkRenderingInfo renderInfo{ .sType = VK_STRUCTURE_TYPE_RENDERING_INFO };
    renderInfo.renderArea           = VkRect2D{ VkOffset2D { 0, 0 }, renderExtent };
    renderInfo.layerCount           = 1;
    renderInfo.colorAttachmentCount = uint32_t(colorAttachments.size());
    renderInfo.pColorAttachments    = colorAttachments.data();
    renderInfo.pDepthAttachment     = depthAttachment;
    renderInfo.pStencilAttachment   = nullptr;

    return renderInfo;
}

VkImageSubresourceRange core::gpu_detail::ImageSubresourceRange(VkImageAspectFlags aspectMask)
{
    VkImageSubresourceRange subImage{};
    subImage.aspectMask     = aspectMask;
    subImage.baseMipLevel   = 0;
    subImage.levelCount     = VK_REMAINING_MIP_LEVELS;
    subImage.baseArrayLayer = 0;
    subImage.layerCount     = VK_REMAINING_ARRAY_LAYERS;

    return subImage;
}

VkShaderModuleCreateInfo core::gpu_detail::ShaderModuleCreateInfo(const void* data, size_t size)
{
    VkShaderModuleCreateInfo result { .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    result.pCode    = reinterpret_cast<const uint32_t*>(data);
    result.codeSize = size;
    return result;
}

VkDescriptorSetLayoutBinding core::gpu_detail::DescriptorSetLayoutBinding(VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t binding)
{
    VkDescriptorSetLayoutBinding setbind = {};
    setbind.binding            = binding;
    setbind.descriptorCount    = 1;
    setbind.descriptorType     = type;
    setbind.pImmutableSamplers = nullptr;
    setbind.stageFlags         = stageFlags;

    return setbind;
}

VkDescriptorSetLayoutCreateInfo core::gpu_detail::DescriptorSetLayoutCreateInfo(VkDescriptorSetLayoutBinding* bindings, uint32_t bindingCount)
{
    VkDescriptorSetLayoutCreateInfo info = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    info.pBindings    = bindings;
    info.bindingCount = bindingCount;
    info.flags        = 0;

    return info;
}

VkWriteDescriptorSet core::gpu_detail::WriteDescriptorImage(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorImageInfo* imageInfo, uint32_t binding)
{
    VkWriteDescriptorSet write = { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    write.dstBinding      = binding;
    write.dstSet          = dstSet;
    write.descriptorCount = 1;
    write.descriptorType  = type;
    write.pImageInfo      = imageInfo;

    return write;
}

VkWriteDescriptorSet core::gpu_detail::WriteDescriptorBuffer(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorBufferInfo* bufferInfo, uint32_t binding)
{
    VkWriteDescriptorSet write = { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    write.dstBinding      = binding;
    write.dstSet          = dstSet;
    write.descriptorCount = 1;
    write.descriptorType  = type;
    write.pBufferInfo     = bufferInfo;

    return write;
}

VkDescriptorBufferInfo core::gpu_detail::BufferInfo(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range)
{
    VkDescriptorBufferInfo binfo = {};
    binfo.buffer = buffer;
    binfo.offset = offset;
    binfo.range  = range;
    return binfo;
}

VkImageCreateInfo core::gpu_detail::ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent)
{
    VkImageCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext = nullptr;

    info.imageType = VK_IMAGE_TYPE_2D;

    info.format = format;
    info.extent = extent;

    info.mipLevels = 1;
    info.arrayLayers = 1;

    info.samples = VK_SAMPLE_COUNT_1_BIT;

    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = usageFlags;

    return info;
}

VkImageViewCreateInfo core::gpu_detail::ImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.pNext = nullptr;

    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.image = image;
    info.format = format;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = 1;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = 1;
    info.subresourceRange.aspectMask = aspectFlags;

    return info;
}

VkPipelineLayoutCreateInfo core::gpu_detail::PipelineLayoutCreateInfo()
{
    VkPipelineLayoutCreateInfo info{ .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    return info;
}

VkPipelineShaderStageCreateInfo core::gpu_detail::PipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shaderModule, const char* entry /* nullptr */)
{
    VkPipelineShaderStageCreateInfo info { .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    info.stage  = stage;
    info.module = shaderModule;
    info.pName  = entry ? entry : "main";
    return info;
}

