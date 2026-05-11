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
#include <graphics/materialInstance.h>
#include <core/gpu/buffer.h>


#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <graphics/assets/mesh.h>



namespace graphics::resources
{
	struct Mesh
	{
		MaterialInstance* GetMaterial(uint32_t index) const
		{
			if (index < materials.size())
				return materials[index].get();
			return nullptr;
		}
		/*Cette partie n'est pas un Asset c'est la partie ressource car c'est du runtime*/
		std::unique_ptr<core::gpu::Buffer> vertexBuffer;
		std::unique_ptr<core::gpu::Buffer> indexBuffer;
		std::unique_ptr<core::gpu::Buffer> rtVertexBuffer;
		std::unique_ptr<core::gpu::Buffer> rtIndexBuffer;
		std::unique_ptr<core::gpu::AccelerationStructure> blas;

		std::vector<std::shared_ptr<MaterialInstance>> materials;
		std::vector<SubMesh>  subMeshes;

		core::gpu::Buffer* GetVertexBuffer()  const { return vertexBuffer.get(); }
		core::gpu::Buffer* GetIndexBuffer()   const { return indexBuffer.get(); }
	};
}

#endif //VRAKTAL_GRAPHICS_RESOURCES_MESH_H