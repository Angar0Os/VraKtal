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
	
	struct MeshInstance : public EntityBase
	{
		MeshInstance() : temp_transform(1.0f) {
            keyframes.push_back({ 0.0f, EInterpolationType::Linear, { 0, glm::mat4(1.0f) } });
		};
		std::vector<Keyframe<MeshProperties>> keyframes;
		graphics::resources::Mesh* mesh = nullptr;
		glm::mat4 temp_transform;
	};
}