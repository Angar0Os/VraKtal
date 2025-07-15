#ifndef VRAKTAL_RHI_MATERIAL_H
#define VRAKTAL_RHI_MATERIAL_H
#pragma once

namespace vk { class VulkanDevice; class VulkanPipeline; class VulkanSwapchain; }

class VulkanEngine;

namespace rhi
{
	struct GLTFMetallic_Roughness
	{
		virtual void BuildPipelines(VulkanEngine* engine, vk::VulkanDevice* device, vk::VulkanPipeline* pipeline, vk::VulkanSwapchain* swapchain) = 0;
	};
}

#endif //VRAKTAL_RHI_MATERIAL_H
