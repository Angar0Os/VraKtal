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
	private:
		core::gpu::Device* m_device;

		std::shared_ptr<graphics::resources::Mesh> LoadGLTF(const std::string& filepath);
		std::shared_ptr<graphics::resources::Mesh> LoadOBJ(const std::string& filepath);
		void CreateBuffersForMesh(graphics::resources::Mesh* mesh);
		void CreateBLASForMesh(graphics::resources::Mesh* mesh);

	public:
		MeshLoader(core::gpu::Device* device);

		std::shared_ptr<graphics::resources::Mesh> LoadMesh(const std::string& filepath);
		std::shared_ptr<graphics::resources::Mesh> CreatePlane(float width, float height, int subdivisionsX = 1, int subdivisionsZ = 1);
	};
}

#endif //VRAKTAL_LOADERS_MESHLOADER_H
