#pragma once
#include <scene/timeline/entityBase.h>
#include <glm/glm.hpp>

#include <graphics/resources/object/light.h>

#include <vector>

namespace timeline {
	enum class LightType
	{
		Point = 0,
		Directional = 1,
		Spot = 2
	};

	struct LightProperty
	{
		LightProperty()	: 
			position(2.0f, 2.0f, 2.0f),
			direction(0.0f, -1.0f, 0.0f),
			color(1.0f, 1.0f, 1.0f),
			intensity(1.0f),
			enabled(true),
			type(LightType::Point),
			innerConeAngle(glm::radians(12.5f)),
			outerConeAngle(glm::radians(17.5f)),
			constant(1.0f),
			linear(0.09f),
			quadratic(0.032f),
			radius(100.0f),
			lightRadius(0.5f)
		{}

		glm::vec3 position;
		glm::vec3 direction;
		glm::vec3 color;
		float intensity;
		bool enabled;
		LightType type;

		float innerConeAngle;
		float outerConeAngle;

		float constant;
		float linear;
		float quadratic;
		float radius;

		float lightRadius;
	};

	struct Light : public ComponentBase
	{
		std::vector<Keyframe<LightProperty>> keyframes;
		LightProperty temp_property;
	};
}