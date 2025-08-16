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

	struct AllocatedBuffer
	{
		VkBuffer buffer;
		VmaAllocation allocation;
		VmaAllocationInfo info;
	};

	struct GPUMeshBuffers
	{
		AllocatedBuffer indexBuffer;
		AllocatedBuffer vertexBuffer;
		VkDeviceAddress vertexBufferAddress;
	};

	struct GPUDrawPushConstants
	{
		glm::mat4 worldMatrix;
		VkDeviceAddress vertexBuffer;
	};

	struct GPUSceneData
	{
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 viewproj;
		glm::vec4 ambientColor;
		glm::vec4 sunlightDirection;
		glm::vec4 sunlightColor;
	};
}

#endif //VRAKTAL_VK_TYPES_H
