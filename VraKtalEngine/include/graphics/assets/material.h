#pragma once

#include <string>
#include <glm/glm.hpp>
namespace graphics {
	enum class MaterialType
	{
		PBR,
		Unlit,
		Skybox
	};
}


namespace graphics::assets
{
	struct Material 
	{
		/* 
			On utilise le material pour savoir ce que l'instance va contenir 
			L'instance est un peu le resultat du chargement du material donc on PEUT
			Stocker les materials dans les ressources.
			On peut aussi stocker les material instances dans les ressources ? 
				non parceque les ressources sont partagees
			Qu'estce que ca veut dire ? 
				On ne veut (surement) pas pouvoir faire :
				entityA.material = resourceManager.Get<MaterialInstance>("wood");
				entityB.material = resourceManager.Get<MaterialInstance>("wood");
				entityA.material->SetBaseColor(red);
				et la on est baisee car le mat de entityB change aussi.
			Donc on doit soit avoir un un runtimeMaterialManager mais que a l'editor
			pour pouvoir update les materiaux des mesh dont on change le parent de l'instance.
		*/

		std::string name;
		std::string albedoTexture;
		std::string normalTexture;
		std::string metallicTexture;
		std::string roughnessTexture;
		std::string aoTexture;
		std::string emissiveTexture;
		MaterialType materialType = graphics::MaterialType::PBR;

		glm::vec3 albedo;
		float metallic;
		float roughness;
		float ao;
		glm::vec3 emissive;
		float emissiveStrength;
		float opacity;
		bool doubleSided;

		Material()
			: name("DefaultMaterial"),
			albedo(1.0f, 1.0f, 1.0f),
			metallic(0.0f),
			roughness(1.0f),
			ao(1.0f),
			emissive(0.0f, 0.0f, 0.0f),
			opacity(1.0f),
			emissiveStrength(1.0f),
			doubleSided(false)
		{
		}

		void SetTexture(const std::string& path, const std::string& type)
		{
			if (type == "albedo" || type == "diffuse" || type == "base_color")
			{
				albedoTexture = path;
			}
			else if (type == "normal")
			{
				normalTexture = path;
			}
			else if (type == "metallic")
			{
				metallicTexture = path;
			}
			else if (type == "roughness")
			{
				roughnessTexture = path;
			}
			else if (type == "ao" || type == "ambient_occlusion")
			{
				aoTexture = path;
			}
			else if (type == "emissive")
			{
				emissiveTexture = path;
			}
		}

		void RemoveTexture(const std::string& type)
		{
			if (type == "albedo" || type == "diffuse" || type == "base_color")
			{
				albedoTexture.clear();
			}
			else if (type == "normal")
			{
				normalTexture.clear();
			}
			else if (type == "metallic")
			{
				metallicTexture.clear();
			}
			else if (type == "roughness")
			{
				roughnessTexture.clear();
			}
			else if (type == "ao" || type == "ambient_occlusion")
			{
				aoTexture.clear();
			}
			else if (type == "emissive")
			{
				emissiveTexture.clear();
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