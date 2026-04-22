#pragma once
#include "../entityBase.h"
#include <cstdint>
#include <vector>

#include <graphics/resources/object/mesh.h>

namespace timeline {

	struct MeshProperties
	{
		MeshProperties() {
			transform = glm::mat4(1.0f);
		};
		glm::mat4 transform;
	};
	
	struct MeshInstance : public ComponentBase
	{
		std::vector<Keyframe<MeshProperties>> keyframes;
		uint32_t meshID = -1;
		MeshProperties temp_properties;
	};

}