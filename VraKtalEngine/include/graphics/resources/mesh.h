#ifndef VRAKTAL_GRAPHICS_RESOURCES_MESH_H
#define VRAKTAL_GRAPHICS_RESOURCES_MESH_H
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

namespace graphics::resources
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;
		glm::vec4 tangent;

		bool operator==(const Vertex& other) const
		{
			return position == other.position &&
				normal == other.normal &&
				uv == other.uv &&
				tangent == other.tangent;
		}
	};

	class Mesh
	{
	public:
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

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
			{
				vertex.position += offset;
			}
		}

		void Scale(const glm::vec3& scale)
		{
			for (auto& vertex : vertices)
			{
				vertex.position *= scale;
			}
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
			{
				vertex.normal = glm::vec3(0.0f);
			}

			for (size_t i = 0; i < indices.size(); i += 3)
			{
				uint32_t i0 = indices[i];
				uint32_t i1 = indices[i + 1];
				uint32_t i2 = indices[i + 2];

				glm::vec3 v0 = vertices[i0].position;
				glm::vec3 v1 = vertices[i1].position;
				glm::vec3 v2 = vertices[i2].position;

				glm::vec3 edge1 = v1 - v0;
				glm::vec3 edge2 = v2 - v0;
				glm::vec3 normal = glm::cross(edge1, edge2);

				vertices[i0].normal += normal;
				vertices[i1].normal += normal;
				vertices[i2].normal += normal;
			}

			for (auto& vertex : vertices)
			{
				if (glm::length(vertex.normal) > 0.0f)
				{
					vertex.normal = glm::normalize(vertex.normal);
				}
			}
		}

		Mesh Clone() const
		{
			Mesh mesh;
			mesh.vertices = vertices;
			mesh.indices = indices;
			return mesh;
		}

		void Clear()
		{
			vertices.clear();
			indices.clear();
		}
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