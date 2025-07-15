#ifndef VRAKTAL_RHI_MATERIAL_H
#define VRAKTAL_RHI_MATERIAL_H
#pragma once

namespace vk { class VulkanEngine; class VulkanDevice; class VulkanPipeline; class VulkanSwapchain; }

namespace rhi
{
	struct GLTFMetallic_Roughness
	{
		virtual void BuildPipelines(vk::VulkanEngine* engine, vk::VulkanDevice* device, vk::VulkanPipeline* pipeline, vk::VulkanSwapchain* swapchain) = 0;
	};
}

#endif //VRAKTAL_RHI_MATERIAL_H
