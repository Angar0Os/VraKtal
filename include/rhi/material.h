#ifndef VRAKTAL_RHI_MATERIAL_H
#define VRAKTAL_RHI_MATERIAL_H
#pragma once

namespace vk { class VulkanEngine; class VkDevice; }

namespace rhi
{
	struct GLTFMetallic_Roughness
	{
		virtual void BuildPipelines(vk::VulkanEngine* engine) = 0;
		virtual void ClearResources(vk::VkDevice device) = 0;
	};
}

#endif //VRAKTAL_RHI_MATERIAL_H
