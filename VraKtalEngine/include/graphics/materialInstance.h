#ifndef VRAKTAL_GRAPHICS_MATERIAL_INSTANCE_H
#define VRAKTAL_GRAPHICS_MATERIAL_INSTANCE_H
#pragma once

#include <core/gpu/device.h>
#include <core/gpu/buffer.h>
#include <core/gpu/image.h>
#include <core/gpu/texture.h>
#include <core/gpu/descriptorSet.h>
#include <core/gpu/descriptorSetLayout.h>

#include <glm/glm.hpp>

#include <memory>
#include <string>

struct MaterialGPUData
{
	glm::vec4 baseColor;
	// rgb = albedoColor, a = unused

	glm::vec4 params;
	// x = metallic
	// y = roughness
	// z = hasAlbedoTexture
	// w = hasNormalTexture
};

namespace graphics::resources
{
	/* 
		On pourrait faire un materialInstanceAsset pour la serialisation
		puisque enfaite si on modifie des valeurs il faut quand meme les sauver (gpuData uniquement)
	*/

	struct MaterialInstance //runtime tres proche du gpu 
	{
		std::string name = "Default";

		std::unique_ptr<core::gpu::Image>   albedoImage;
		std::unique_ptr<core::gpu::Texture> albedoTexture;

		std::unique_ptr<core::gpu::Image>   normalImage;
		std::unique_ptr<core::gpu::Texture> normalTexture;

		std::unique_ptr<core::gpu::Image>   roughnessMetalImage;
		std::unique_ptr<core::gpu::Texture> roughnessMetalTexture;

		glm::vec3 albedoColor = { 1.0f, 1.0f, 1.0f };
		float     metalness = 0.0f;
		float     roughnessValue = 1.0f;

		bool hasAlbedoTexture = false;
		bool hasNormalTexture = false;
		bool hasRoughnessMetalTexture = false;

		std::unique_ptr<core::gpu::DescriptorSet> descriptorSet;
		std::unique_ptr<core::gpu::Buffer> materialBuffer;
		MaterialGPUData gpuData;

		size_t index; // used by materialInstanceManager to remove on destruction

		void SetBaseColor(const glm::vec4& color)
		{
			gpuData.baseColor = color;

			materialBuffer->CopyFrom(
				&gpuData,
				sizeof(MaterialGPUData)
			);
		}

		void SetRoughness(const float& _roughness)
		{
			gpuData.params.y = _roughness;

			materialBuffer->CopyFrom(
				&gpuData,
				sizeof(MaterialGPUData)
			);
		}

		void SetMetalness(const float& _metalness)
		{
			gpuData.params.x = _metalness;

			materialBuffer->CopyFrom(
				&gpuData,
				sizeof(MaterialGPUData)
			);
		}
	};


}

#endif //VRAKTAL_GRAPHICS_MATERIAL_INSTANCE_H
