#ifndef VRAKTAL_GRAPHICS_MATERIAL_INSTANCE_H
#define VRAKTAL_GRAPHICS_MATERIAL_INSTANCE_H
#pragma once

#include <core/gpu/device.h>
#include <core/gpu/image.h>
#include <core/gpu/texture.h>
#include <core/gpu/descriptorSet.h>
#include <core/gpu/descriptorSetLayout.h>

#include <glm/glm.hpp>

#include <memory>
#include <string>

namespace graphics::resources
{
	struct MaterialInstance
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
	};
}

#endif //VRAKTAL_GRAPHICS_MATERIAL_INSTANCE_H
