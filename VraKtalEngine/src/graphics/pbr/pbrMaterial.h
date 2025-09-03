#ifndef VRAKTAL_SRC_GRAPHICS_PBR_PBR_MATERIAL_H
#define VRAKTAL_SRC_GRAPHICS_PBR_PBR_MATERIAL_H
#pragma once

#include <glm/glm.hpp>
#include <string>

namespace graphics::pbr
{
	struct PBRMaterial
	{
		glm::vec4 baseColorFactor{ 1.0f };
		float metallicColor{ 1.0f };
		float roughnessFactor{ 1.0f };
		glm::vec3 emissiveFactor{ 0.0f };

		std::string baseColorTexture;
		std::string metallicRoughnessTexture;
		std::string normalTexture;
		std::string occlusionTexture;
		std::string emissiveTexture;
	};
}

#endif //VRAKTAL_SRC_GRAPHICS_PBR_PBR_MATERIAL_H
