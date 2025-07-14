#ifndef VRAKTAL_VK_IMAGE_H
#define VRAKTAL_VK_IMAGE_H
#pragma once

#include <vk/image.h>
#include <vma/vk_mem_alloc.h>

namespace vk
{
	struct AllocatedBuffer
	{
		VkBuffer buffer;
		VmaAllocation allocation;
		VmaAllocationInfo info;
	};


	class VulkanBuffer
	{
	public:
		VulkanBuffer() = default;

		VulkanBuffer(VulkanImage vkImage)
			: _vkImage(vkImage) {
		}

		void Destroy(const AllocatedBuffer& buffer);
		AllocatedBuffer CreateBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

	private:
		VulkanImage _vkImage;
	};
}

#endif // VRAKTAL_VK_IMAGE_H