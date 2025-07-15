#ifndef VRAKTAL_VK_MATERIAL_H
#define VRAKTAL_VK_MATERIAL_H
#pragma once

#include <iostream>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <rhi/material.h>

namespace vk
{
	class AllocatedImage;
	class DescriptorWriter;
	class DescriptorAllocatorGrowable;

	enum class MaterialPass : uint8_t
	{
		MainColor,
		Transparent,
		Other
	};

	struct MaterialPipeline
	{
		VkPipeline pipeline;
		VkPipelineLayout layout;
	};

	struct MaterialInstance
	{
		MaterialPipeline* pipeline;
		VkDescriptorSet materialSet;
		MaterialPass passType;
	};

	struct GLTFMetallic_Roughness : public rhi::GLTFMetallic_Roughness
	{
		MaterialPipeline opaquePipeline;
		MaterialPipeline transparentPipeline;

		VkDescriptorSetLayout materialLayout;

		struct MaterialConstants {
			glm::vec4 colorFactors;
			glm::vec4 metal_rough_factors;
			glm::vec4 extra[14];
		};

		struct MaterialResources {
			AllocatedImage* colorImage;
			VkSampler colorSampler;
			AllocatedImage* metalRoughImage;
			VkSampler metalRoughSampler;
			VkBuffer dataBuffer;
			uint32_t dataBufferOffset;
		};

		DescriptorWriter* writer;

		void BuildPipelines(VulkanEngine* engine) override;
		void ClearResources(VkDevice device) override;

		MaterialInstance write_material(VkDevice device, MaterialPass pass, const MaterialResources& resources, DescriptorAllocatorGrowable& descriptorAllocator);
	};

}

#endif //VRAKTAL_VK_MATERIAL_H


