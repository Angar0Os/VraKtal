#ifndef VRAKTAL_VULKAN_IMAGE_H
#define VRAKTAL_VULKAN_IMAGE_H
#pragma once

#include <rhi/image.h>
#include <vma/vk_mem_alloc.h>

namespace vk
{
	class VulkanCommandBuffer;
	class VulkanBuffer;

	struct AllocatedImage
	{
		VkImage image;
		VkImageView imageView;
		VmaAllocation allocation;
		VkExtent3D imageExtent;
		VkFormat imageFormat;
	};

	class VulkanImage : public rhi::Image
	{
	public:
		explicit VulkanImage() = default;

		explicit VulkanImage(VkDevice device, VmaAllocator allocator, VulkanBuffer* buffer, VulkanCommandBuffer* commandBuffer)
			: _device(device), _allocator(allocator), _buffer(buffer), _commandBuffer(commandBuffer) {
		}

		VkImageCreateInfo CreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent) override;
		VkImageViewCreateInfo CreateViewInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags) override;
		VkImageSubresourceRange CreateInfoSubresourceRange(VkImageAspectFlags aspectMask) override;
		void TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout) override;
		void CopyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize) override;

		AllocatedImage CreateImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);
		AllocatedImage CreateImage(void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);
		void DestroyImage(const AllocatedImage& img);

		VmaAllocator GetAllocator() const { return _allocator; }
		VkDevice GetDevice() const { return _device; }

	private:
		VkDevice _device = VK_NULL_HANDLE;
		VmaAllocator _allocator = VK_NULL_HANDLE;
		VulkanBuffer* _buffer;
		VulkanCommandBuffer* _commandBuffer;
	};
}

#endif // VRAKTAL_VULKAN_IMAGE_H
