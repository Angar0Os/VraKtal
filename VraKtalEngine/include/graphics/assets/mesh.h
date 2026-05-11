#pragma once
#include <graphics/assets/asset.h>

#include <string>
#include <glm/glm.hpp>
#include <vector>



namespace graphics {
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

	namespace assets {
		struct Mesh : public Asset
		{
			std::string name = "mesh";
			std::string path = "undefined";

			std::vector<Vertex>   vertices;
			std::vector<uint32_t> indices;
			std::vector<SubMesh>  subMeshes;
			uint32_t indexCount = 0; // ? 

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

			uint32_t           GetVertexCount()   const { return static_cast<uint32_t>(vertices.size()); }
			uint32_t           GetIndexCount()    const { return static_cast<uint32_t>(indices.size()); }

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
		};
	}
}

namespace std
{
	template<>
	struct hash<graphics::Vertex>
	{
		size_t operator()(const graphics::Vertex& vertex) const
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