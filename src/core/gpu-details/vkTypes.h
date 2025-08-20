#ifndef VRAKTAL_VK_TYPES_H
#define VRAKTAL_VK_TYPES_H
#pragma once

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

#include "../../graphics/material_impl_vulkan.h"

#include <deque>
#include <functional>
#include <glm/glm.hpp>

namespace vkTypes
{
	struct MaterialInstance;
	
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

	enum class MaterialPass : uint8_t
	{
		MainColor,
		Transparent,
		Other
	};

	struct Bounds
	{
		glm::vec3 origin;
		float sphereRadius;
		glm::vec3 extents;
	};

	struct Vertex
	{
		glm::vec3 position;
		float uv_x;
		glm::vec3 normal;
		float uv_y;
		glm::vec4 color;
	};

	struct RenderObject
	{
		uint32_t indexCount;
		uint32_t firstIndex;
		VkBuffer indexBuffer;

		MaterialInstance* material;

		Bounds bounds;
		glm::mat4 transform;
		VkDeviceAddress vertexBufferAddress;
	};
	
	struct DrawContext
	{
		std::vector<RenderObject> OpaqueSurfaces;
		std::vector<RenderObject> TransparentSurfaces;
		std::unordered_map<MaterialPass, VkPipeline> pipelines;
		MaterialPass activePass = MaterialPass::MainColor;
	};
}

#endif //VRAKTAL_VK_TYPES_H
