#ifndef VRAKTAL_GPU_DETAILS_VK_INITIALIZERS_H
#define VRAKTAL_GPU_DETAILS_VK_INITIALIZERS_H
#pragma once

#include <vector>

#include "vkTypes.h"
#include <vulkan/vulkan.h>

namespace core::gpu_detail
{
    VkCommandPoolCreateInfo         CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
    VkCommandBufferAllocateInfo     CommandBufferAllocateInfo(VkCommandPool pool, bool isSecondary = false, uint32_t count = 1);
    VkCommandBufferBeginInfo        CommandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0);
    VkCommandBufferSubmitInfo       CommandBufferSubmitInfo(VkCommandBuffer cmd);
    VkFenceCreateInfo               FenceCreateInfo(VkFenceCreateFlags flags = 0);
    VkSemaphoreCreateInfo           SemaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);
    VkSubmitInfo2                   SubmitInfo(VkCommandBufferSubmitInfo* cmd, const std::vector<VkSemaphoreSubmitInfo>& signalSemaphoreInfos = {}, const std::vector<VkSemaphoreSubmitInfo>& waitSemaphoreInfos = {});
    VkPresentInfoKHR                PresentInfo(const VkSwapchainKHR& swapchain, const VkSemaphore& waitSemaphore, const uint32_t& imageIndex);
    VkRenderingAttachmentInfo       AttachmentInfo(VkImageView view, VkClearValue* clear, VkImageLayout layout /*= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL*/);
    VkRenderingAttachmentInfo       DepthAttachmentInfo(VkImageView view, VkImageLayout layout /*= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL*/);
    VkRenderingInfo                 RenderingInfo(VkExtent2D renderExtent, const std::vector<VkRenderingAttachmentInfo>& colorAttachments, VkRenderingAttachmentInfo* depthAttachment);
    VkImageSubresourceRange         ImageSubresourceRange(VkImageAspectFlags aspectMask);
    VkSemaphoreSubmitInfo           SemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);
    VkShaderModuleCreateInfo        ShaderModuleCreateInfo(const void* data, size_t size);
    VkDescriptorSetLayoutBinding    DescriptorSetLayoutBinding(VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t binding);
    VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo(VkDescriptorSetLayoutBinding* bindings, uint32_t bindingCount);
    VkWriteDescriptorSet            WriteDescriptorImage(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorImageInfo* imageInfo, uint32_t binding);
    VkWriteDescriptorSet            WriteDescriptorBuffer(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorBufferInfo* bufferInfo, uint32_t binding);
    VkDescriptorBufferInfo          BufferInfo(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);
    VkImageCreateInfo               ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
    VkImageViewCreateInfo           ImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
    VkPipelineLayoutCreateInfo      PipelineLayoutCreateInfo();
    VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shaderModule, const char* entry = nullptr);
}

#endif //VRAKTAL_GPU_DETAILS_VK_INITIALIZERS_H