#ifndef VRAKTAL_GRAPHICS_RESOURCES_MATERIAL_H
#define VRAKTAL_GRAPHICS_RESOURCES_MATERIAL_H
#pragma once

#include <string>
#include <glm/glm.hpp>

namespace graphics::resources
{
	struct Material
	{
		std::string name;

		std::string albedoTexture;
		std::string normalTexture;
		std::string metallicTexture;
		std::string roughnessTexture;
		std::string aoTexture;
		std::string emissiveTexture;

		glm::vec3 albedo;
		float metallic;
		float roughness;
		float ao;
		glm::vec3 emissive;

		bool useAlbedoTexture;
		bool useNormalTexture;
		bool useMetallicTexture;
		bool useRoughnessTexture;
		bool useAOTexture;
		bool useEmissiveTexture;

		Material()
			: name("DefaultMaterial"),	
			  albedo(1.0f, 1.0f, 1.0f),
			  metallic(0.0f),
			  roughness(1.0f),
			  ao(1.0f),
			  emissive(0.0f, 0.0f, 0.0f),
			  useAlbedoTexture(false),
			  useNormalTexture(false),
			  useMetallicTexture(false),
			  useRoughnessTexture(false),
			  useAOTexture(false),
			  useEmissiveTexture(false)
		{
		}
	};
}

#endif //VRAKTAL_GRAPHICS_RESOURCES_MATERIAL_H
