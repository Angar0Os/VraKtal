#ifndef VRAKTAL_GRAPHICS_RESOURCES_MESH_H
#define VRAKTAL_GRAPHICS_RESOURCES_MESH_H
#pragma once

#include <glm/glm.hpp>
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
