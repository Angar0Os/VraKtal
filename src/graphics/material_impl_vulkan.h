#ifndef VRAKTAL_GRAPHICS_MATERIAL_IMPL_VULKAN_H
#define VRAKTAL_GRAPHICS_MATERIAL_IMPL_VULKAN_H
#pragma once

#include <graphics/material.h>
#include <glm/glm.hpp>

#include "../core/gpu-details/vkTypes.h"
#include "../core/gpu/descriptor_impl_vulkan.h"
#include "../core/renderContext_impl_glfw_vulkan.h"


struct MaterialInstance;

struct MaterialPipeline
{
	VkPipeline pipeline;
	VkPipelineLayout layout;
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	VkDescriptorSetLayout descriptorSetLayout = nullptr;
};

struct MaterialInstance
{
	MaterialPipeline* pipeline;
	VkDescriptorSet materialSet;
	vkTypes::MaterialPass passType;
};

struct GPUDrawPushConstants
{
	glm::mat4 worldMatrix;
	VkDeviceAddress vertexBuffer;
};

struct GLTFMetallic_Roughness
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
		vkTypes::AllocatedImage colorImage;
		VkSampler colorSampler;
		vkTypes::AllocatedImage metalRoughImage;
		VkSampler metalRoughSampler;
		VkBuffer dataBuffer;
		uint32_t dataBufferOffset;
	};

	DescriptorWriter writer;

	void BuildPipelines(core::rhi::RenderContext& renderContext);
	void ClearResources(VkDevice device);

	MaterialInstance WriteMaterial(VkDevice device, vkTypes::MaterialPass pass, const MaterialResources& resources, DescriptorAllocatorGrowable& descriptorAllocator);
};

struct graphics::rhi::Material::Internal
{
	GLTFMetallic_Roughness metalRoughMaterial;
};

#endif //VRAKTAL_GRAPHICS_MATERIAL_IMPL_VULKAN_H
