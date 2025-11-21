#ifndef VRAKTAL_GRAPHICS_RESOURCES_SCENE_H
#define VRAKTAL_GRAPHICS_RESOURCES_SCENE_H
#pragma once

#include <graphics/resources/mesh.h>
#include <glm/glm.hpp>

namespace graphics::resources
{
	class Scene
	{
	public:
		struct MeshInstance
		{
			std::shared_ptr<Mesh> mesh;
			glm::mat4 transform;
		};

		std::vector<MeshInstance> meshInstances;

		MeshInstance AddMesh(std::shared_ptr<Mesh> mesh, const glm::mat4& transform = glm::mat4(1.0f))
		{
			MeshInstance instance = { mesh, transform };
			meshInstances.push_back(instance);
			return instance;
		}
	};
}

#endif //VRAKTAL_GRAPHICS_RESOURCES_SCENE_H
