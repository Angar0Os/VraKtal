#ifndef VRAKTAL_GRAPHICS_RESOURCES_SCENE_H
#define VRAKTAL_GRAPHICS_RESOURCES_SCENE_H
#pragma once

#include <graphics/resources/mesh.h>
#include <graphics/resources/material.h>
#include <graphics/resources/light.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <memory>
#include <vector>
#include <string>

namespace graphics::resources
{
	class Scene
	{
	public:
		struct MeshInstance
		{
			std::shared_ptr<Mesh> mesh;
			std::shared_ptr<Material> material;
			glm::mat4 transform;
			std::string name;
			bool visible = true;

			glm::vec3 GetPosition() const
			{
				return glm::vec3(transform[3]);
			}

			void SetPosition(const glm::vec3& position)
			{
				transform[3] = glm::vec4(position, 1.0f);
			}

			void Translate(const glm::vec3& offset)
			{
				transform = glm::translate(transform, offset);
			}

			void Rotate(float angle, const glm::vec3& axis)
			{
				transform = glm::rotate(transform, angle, axis);
			}

			void Scale(const glm::vec3& scale)
			{
				transform = glm::scale(transform, scale);
			}
		};

		std::vector<MeshInstance> meshInstances;
		std::vector<Light> lights;

		MeshInstance* AddMesh(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material,
			const glm::mat4& transform = glm::mat4(1.0f), const std::string& name = "")
		{
			MeshInstance instance = { mesh, material, transform, name };
			meshInstances.push_back(instance);
			return &meshInstances.back();
		}

		void ClearMeshes()
		{
			meshInstances.clear();
		}

		void RemoveMesh(MeshInstance& instance)
		{
			auto it = std::remove_if(
				meshInstances.begin(),
				meshInstances.end(),
				[&instance](const MeshInstance& mi)
				{
					return mi.mesh == instance.mesh && mi.material == instance.material && mi.transform == instance.transform;
				}
			);
			if (it != meshInstances.end())
			{
				meshInstances.erase(it, meshInstances.end());
			}
		}

		void RemoveMeshByIndex(size_t index)
		{
			if (index < meshInstances.size())
			{
				meshInstances.erase(meshInstances.begin() + index);
			}
		}

		void RemoveMeshByName(const std::string& name)
		{
			auto it = std::remove_if(
				meshInstances.begin(),
				meshInstances.end(),
				[&name](const MeshInstance& mi)
				{
					return mi.name == name;
				}
			);
			meshInstances.erase(it, meshInstances.end());
		}

		MeshInstance* FindMeshByName(const std::string& name)
		{
			auto it = std::find_if(
				meshInstances.begin(),
				meshInstances.end(),
				[&name](const MeshInstance& mi)
				{
					return mi.name == name;
				}
			);
			return (it != meshInstances.end()) ? &(*it) : nullptr;
		}

		size_t GetMeshCount() const
		{
			return meshInstances.size();
		}

		size_t GetVisibleMeshCount() const
		{
			return std::count_if(
				meshInstances.begin(),
				meshInstances.end(),
				[](const MeshInstance& mi) { return mi.visible; }
			);
		}

		void SetAllMeshesVisible(bool visible)
		{
			for (auto& instance : meshInstances)
			{
				instance.visible = visible;
			}
		}

		void AddLight(const Light& light)
		{
			lights.push_back(light);
		}

		void RemoveLight(size_t index)
		{
			if (index < lights.size())
			{
				lights.erase(lights.begin() + index);
			}
		}

		void RemoveLightByName(const std::string& name)
		{
			auto it = std::remove_if(
				lights.begin(),
				lights.end(),
				[&name](const Light& light)
				{
					return light.name == name;
				}
			);
			lights.erase(it, lights.end());
		}

		void ClearLights()
		{
			lights.clear();
		}

		Light* FindLightByName(const std::string& name)
		{
			auto it = std::find_if(
				lights.begin(),
				lights.end(),
				[&name](const Light& light)
				{
					return light.name == name;
				}
			);
			return (it != lights.end()) ? &(*it) : nullptr;
		}

		size_t GetLightCount() const
		{
			return lights.size();
		}

		size_t GetEnabledLightCount() const
		{
			return std::count_if(
				lights.begin(),
				lights.end(),
				[](const Light& light) { return light.enabled; }
			);
		}

		void SetAllLightsEnabled(bool enabled)
		{
			for (auto& light : lights)
			{
				light.enabled = enabled;
			}
		}

		void Clear()
		{
			ClearMeshes();
			ClearLights();
		}
	};
}

#endif //VRAKTAL_GRAPHICS_RESOURCES_SCENE_H