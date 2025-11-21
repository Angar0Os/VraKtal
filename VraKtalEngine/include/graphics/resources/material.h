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

		float opacity;
		float emissiveStrength;
		bool doubleSided;

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
			useEmissiveTexture(false),
			opacity(1.0f),
			emissiveStrength(1.0f),
			doubleSided(false)
		{
		}

		void SetAlbedo(float r, float g, float b)
		{
			albedo = glm::vec3(r, g, b);
		}

		void SetAlbedo(const glm::vec3& color)
		{
			albedo = color;
		}

		void SetEmissive(float r, float g, float b, float strength = 1.0f)
		{
			emissive = glm::vec3(r, g, b);
			emissiveStrength = strength;
		}

		void SetEmissive(const glm::vec3& color, float strength = 1.0f)
		{
			emissive = color;
			emissiveStrength = strength;
		}

		void SetMetallicRoughness(float metal, float rough)
		{
			metallic = glm::clamp(metal, 0.0f, 1.0f);
			roughness = glm::clamp(rough, 0.0f, 1.0f);
		}

		void SetTexture(const std::string& path, const std::string& type)
		{
			if (type == "albedo" || type == "diffuse" || type == "base_color")
			{
				albedoTexture = path;
				useAlbedoTexture = true;
			}
			else if (type == "normal")
			{
				normalTexture = path;
				useNormalTexture = true;
			}
			else if (type == "metallic")
			{
				metallicTexture = path;
				useMetallicTexture = true;
			}
			else if (type == "roughness")
			{
				roughnessTexture = path;
				useRoughnessTexture = true;
			}
			else if (type == "ao" || type == "ambient_occlusion")
			{
				aoTexture = path;
				useAOTexture = true;
			}
			else if (type == "emissive")
			{
				emissiveTexture = path;
				useEmissiveTexture = true;
			}
		}

		void RemoveTexture(const std::string& type)
		{
			if (type == "albedo" || type == "diffuse" || type == "base_color")
			{
				albedoTexture.clear();
				useAlbedoTexture = false;
			}
			else if (type == "normal")
			{
				normalTexture.clear();
				useNormalTexture = false;
			}
			else if (type == "metallic")
			{
				metallicTexture.clear();
				useMetallicTexture = false;
			}
			else if (type == "roughness")
			{
				roughnessTexture.clear();
				useRoughnessTexture = false;
			}
			else if (type == "ao" || type == "ambient_occlusion")
			{
				aoTexture.clear();
				useAOTexture = false;
			}
			else if (type == "emissive")
			{
				emissiveTexture.clear();
				useEmissiveTexture = false;
			}
		}

		void ClearAllTextures()
		{
			albedoTexture.clear();
			normalTexture.clear();
			metallicTexture.clear();
			roughnessTexture.clear();
			aoTexture.clear();
			emissiveTexture.clear();

			useAlbedoTexture = false;
			useNormalTexture = false;
			useMetallicTexture = false;
			useRoughnessTexture = false;
			useAOTexture = false;
			useEmissiveTexture = false;
		}

		bool HasTextures() const
		{
			return useAlbedoTexture || useNormalTexture || useMetallicTexture ||
				useRoughnessTexture || useAOTexture || useEmissiveTexture;
		}

		Material Clone(const std::string& newName = "") const
		{
			Material mat = *this;
			if (!newName.empty())
				mat.name = newName;
			return mat;
		}
	};
}

#endif //VRAKTAL_GRAPHICS_RESOURCES_MATERIAL_H