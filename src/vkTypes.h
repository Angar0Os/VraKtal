#ifndef VRAKTAL_VK_TYPES_H
#define VRAKTAL_VK_TYPES_H
#pragma once

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

namespace vkTypes
{
	struct AllocatedImage
	{
		VkImage image;
		VkImageView imageView;
		VmaAllocation allocation;
		VkExtent3D imageExtent;
		VkFormat imageFormat;
	};
}

#endif //VRAKTAL_VK_TYPES_H
