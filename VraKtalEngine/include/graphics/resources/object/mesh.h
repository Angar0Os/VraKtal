#ifndef VRAKTAL_GRAPHICS_RESOURCES_MESH_H
#define VRAKTAL_GRAPHICS_RESOURCES_MESH_H
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <core/gpu/accelerationStructure.h>
#include <vector>

namespace core::gpu
{
	class Buffer;
}

#pragma once

#include <core/gpu/accelerationStructure.h>
#include <graphics/resources/material.h>
#include <core/gpu/buffer.h>

#include <graphics/assets/mesh.h>

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>



namespace graphics::resources
{
	struct Mesh
	{
		Material& GetMaterial(uint32_t index)
		{
			if (index < materials.size())
				return materials[index];
			throw std::runtime_error("Mesh::GetMaterial(index): index not found in std::vector<Material> materials");
		}
		/*Cette partie n'est pas un Asset c'est la partie ressource car c'est du runtime*/
		std::unique_ptr<core::gpu::Buffer> vertexBuffer;
		std::unique_ptr<core::gpu::Buffer> indexBuffer;
		std::unique_ptr<core::gpu::Buffer> rtVertexBuffer;
		std::unique_ptr<core::gpu::Buffer> rtIndexBuffer;
		std::unique_ptr<core::gpu::AccelerationStructure> blas;

		std::vector<Material> materials;
		std::vector<SubMesh>  subMeshes;

		core::gpu::Buffer* GetVertexBuffer()  const { return vertexBuffer.get(); }
		core::gpu::Buffer* GetIndexBuffer()   const { return indexBuffer.get(); }
	};
}

#endif //VRAKTAL_GRAPHICS_RESOURCES_MESH_H