#ifndef VRAKTAL_LOADERS_MESHLOADER_H
#define VRAKTAL_LOADERS_MESHLOADER_H
#pragma once

#include <memory>
#include <string>

#include <graphics/resources/object/mesh.h>
#include <core/gpu/buffer.h>

namespace loaders
{
	class MeshLoader
	{
	public:
		std::shared_ptr<graphics::resources::object::Mesh> LoadMesh(const std::string& filepath);

		static std::shared_ptr<graphics::resources::object::Mesh> CreatePlane(float width = 10.0f, float height = 10.0f, int subdivisionsX = 1, int subdivisionsZ = 1);
	private:
		std::shared_ptr<graphics::resources::object::Mesh> LoadGLTF(const std::string& filepath);
		std::shared_ptr<graphics::resources::object::Mesh> LoadOBJ(const std::string& filepath);
	};
}

#endif //VRAKTAL_LOADERS_MESHLOADER_H
