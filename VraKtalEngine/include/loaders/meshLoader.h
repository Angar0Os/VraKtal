#ifndef VRAKTAL_LOADERS_MESHLOADER_H
#define VRAKTAL_LOADERS_MESHLOADER_H
#pragma once

#include <memory>
#include <string>
#include <graphics/resources/mesh.h>

namespace loaders
{
	class MeshLoader
	{
	public:
		std::shared_ptr<graphics::resources::Mesh> LoadMesh(const std::string& filepath);

	private:
		std::shared_ptr<graphics::resources::Mesh> LoadGLTF(const std::string& filepath);
		std::shared_ptr<graphics::resources::Mesh> LoadOBJ(const std::string& filepath);
	};
}

#endif //VRAKTAL_LOADERS_MESHLOADER_H
