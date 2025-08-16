#ifndef VRAKTAL_CORE_GPU_IMAGE_IMPL_VULKAN_H
#define VRAKTAL_CORE_GPU_IMAGE_IMPL_VULKAN_H
#pragma once

#include <core/gpu/image.h>
#include <core/renderContext.h>
#include "../src/vkTypes.h"

#include <vulkan/vulkan.h>

struct core::rhi::Image::Internal
{
	void TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
	void CopyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize);
	VkImageSubresourceRange	ImageSubresourceRange(VkImageAspectFlags aspectMask);
	VkImageCreateInfo		ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
	VkImageViewCreateInfo	ImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
	vkTypes::AllocatedImage CreateImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, RenderContext& rCtx, bool mipmapped = false);
	vkTypes::AllocatedImage CreateImage(void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, RenderContext& rCtx, bool mipmapped = false);
	void DestroyImage(const vkTypes::AllocatedImage& img, RenderContext& rCtx);
};

#endif //VRAKTAL_CORE_GPU_IMAGE_IMPL_VULKAN_H
