#ifndef VRAKTAL_VK_TYPES_H
#define VRAKTAL_VK_TYPES_H
#pragma once

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

#include <deque>
#include <functional>

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

	struct DeletionQueue
	{
		std::deque<std::function<void()>> deletors;

		void PushFunction(std::function<void()>&& function)
		{
			deletors.push_back(function);
		}

		void Flush()
		{
			for (auto it = deletors.rbegin(); it != deletors.rend(); ++it)
			{
				(*it)();
			}

			deletors.clear();
		}
	};
}

#endif //VRAKTAL_VK_TYPES_H
