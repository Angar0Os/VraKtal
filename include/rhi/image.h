#ifndef VRAKTAL_RHI_IMAGE_H
#define VRAKTAL_RHI_IMAGE_H
#pragma once

#include <../../src/vkb/VkBootstrap.h>

namespace rhi
{
	class Image
	{
	public:
		virtual VkImageCreateInfo CreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent) = 0;
		virtual VkImageViewCreateInfo CreateViewInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags) = 0;
		virtual VkImageSubresourceRange CreateInfoSubresourceRange(VkImageAspectFlags aspectMask) = 0;
		virtual void TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout) = 0;
		virtual void CopyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize) = 0;
	};
}

#endif // VRAKTAL_RHI_IMAGE_H
