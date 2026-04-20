#pragma once
#include "../entityBase.h"
#include <cstdint>
#include <vector>

#include <graphics/resources/object/mesh.h>

namespace timeline {

	struct MeshProperties
	{
		uint32_t MeshId;
		glm::mat4 transform;
	};
	
	struct Mesh : public EntityBase
	{
		std::vector<Keyframe<MeshProperties>> Keyframes;
		graphics::resources::Mesh* mesh = nullptr;
	};
}