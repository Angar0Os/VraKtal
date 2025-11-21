#ifndef VRAKTAL_GRAPHICS_RESOURCES_SCENE_H
#define VRAKTAL_GRAPHICS_RESOURCES_SCENE_H
#pragma once

#include <graphics/resources/mesh.h>
#include <graphics/resources/material.h>
#include <graphics/resources/light.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

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
		};

		std::vector<MeshInstance> meshInstances;
		std::vector<Light> lights;

		MeshInstance AddMesh(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material, const glm::mat4& transform = glm::mat4(1.0f))
		{
			MeshInstance instance = { mesh, material, transform };
			meshInstances.push_back(instance);
			return instance;
		}

		void AddLight(const Light& light)
		{
			lights.push_back(light);
		}
	};
}

#endif //VRAKTAL_GRAPHICS_RESOURCES_SCENE_H
