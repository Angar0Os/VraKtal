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

#include <graphics/materialInstance.h>

#include <memory>

namespace core::gpu
{
	class Buffer;
}

namespace graphics::resources
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec3 normal;
		glm::vec4 tangent;

		bool operator==(const Vertex& other) const
		{
			return position == other.position &&
				normal == other.normal &&
				uv == other.uv &&
				tangent == other.tangent;
		}
	};

	struct SubMesh
	{
		uint32_t firstIndex = 0;
		uint32_t indexCount = 0;
		uint32_t vertexOffset = 0;
		uint32_t materialIndex = 0;
		std::string name;

		uint32_t GetTriangleCount() const { return indexCount / 3; }
	};

	class Mesh
	{
	public:
		std::vector<Vertex>   vertices;
		std::vector<uint32_t> indices;
		std::vector<SubMesh>  subMeshes;
		std::string sourcePath;

		std::unique_ptr<core::gpu::Buffer> vertexBuffer;
		std::unique_ptr<core::gpu::Buffer> indexBuffer;
		uint32_t indexCount = 0;

		std::unique_ptr<core::gpu::Buffer> rtVertexBuffer;
		std::unique_ptr<core::gpu::Buffer> rtIndexBuffer;
		std::unique_ptr<core::gpu::AccelerationStructure> blas;

		std::vector<std::shared_ptr<MaterialInstance>> materials;

		MaterialInstance* GetMaterial(uint32_t index) const
		{
			if (index < materials.size())
				return materials[index].get();
			return nullptr;
		}

		std::vector<SubMesh> GetSubmeshes() const
		{
			if (!subMeshes.empty())
				return subMeshes;

			SubMesh defaultSubmesh;
			defaultSubmesh.firstIndex = 0;
			defaultSubmesh.indexCount = static_cast<uint32_t>(indices.size());
			defaultSubmesh.vertexOffset = 0;
			defaultSubmesh.materialIndex = 0;
			defaultSubmesh.name = "default";
			return { defaultSubmesh };
		}

		bool HasSubmeshes() const { return !subMeshes.empty(); }

		void Transform(const glm::mat4& matrix)
		{
			glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(matrix)));
			for (auto& vertex : vertices)
			{
				vertex.position = glm::vec3(matrix * glm::vec4(vertex.position, 1.0f));
				vertex.normal = glm::normalize(normalMatrix * vertex.normal);
			}
		}

		void Translate(const glm::vec3& offset)
		{
			for (auto& vertex : vertices)
				vertex.position += offset;
		}

		void Scale(const glm::vec3& scale)
		{
			for (auto& vertex : vertices)
				vertex.position *= scale;
			RecalculateNormals();
		}

		void Rotate(float angle, const glm::vec3& axis)
		{
			glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, axis);
			Transform(rotation);
		}

		void RecalculateNormals()
		{
			for (auto& vertex : vertices)
				vertex.normal = glm::vec3(0.0f);

			for (size_t i = 0; i < indices.size(); i += 3)
			{
				uint32_t i0 = indices[i];
				uint32_t i1 = indices[i + 1];
				uint32_t i2 = indices[i + 2];

				glm::vec3 edge1 = vertices[i1].position - vertices[i0].position;
				glm::vec3 edge2 = vertices[i2].position - vertices[i0].position;
				glm::vec3 normal = glm::cross(edge1, edge2);

				vertices[i0].normal += normal;
				vertices[i1].normal += normal;
				vertices[i2].normal += normal;
			}

			for (auto& vertex : vertices)
				if (glm::length(vertex.normal) > 0.0f)
					vertex.normal = glm::normalize(vertex.normal);
		}

		Mesh Clone() const
		{
			Mesh mesh;
			mesh.vertices = vertices;
			mesh.indices = indices;
			mesh.subMeshes = subMeshes;
			return mesh;
		}

		void Clear()
		{
			vertices.clear();
			indices.clear();
			subMeshes.clear();
			materials.clear();
			vertexBuffer.reset();
			indexBuffer.reset();
		}

		core::gpu::Buffer* GetVertexBuffer()  const { return vertexBuffer.get(); }
		core::gpu::Buffer* GetIndexBuffer()   const { return indexBuffer.get(); }
		uint32_t           GetVertexCount()   const { return static_cast<uint32_t>(vertices.size()); }
		uint32_t           GetIndexCount()    const { return static_cast<uint32_t>(indices.size()); }
	};
}

namespace std
{
	template<>
	struct hash<graphics::resources::Vertex>
	{
		size_t operator()(const graphics::resources::Vertex& vertex) const
		{
			size_t h1 = hash<float>()(vertex.position.x);
			size_t h2 = hash<float>()(vertex.position.y);
			size_t h3 = hash<float>()(vertex.position.z);
			size_t h4 = hash<float>()(vertex.normal.x);
			size_t h5 = hash<float>()(vertex.uv.x);
			size_t h6 = hash<float>()(vertex.uv.y);

			return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4) ^ (h6 << 5);
		}
	};
}

#endif //VRAKTAL_GRAPHICS_RESOURCES_MESH_H