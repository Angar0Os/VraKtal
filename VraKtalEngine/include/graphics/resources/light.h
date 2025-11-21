#ifndef VRAKTAL_GRAPHICS_RESOURCES_LIGHT_H
#define VRAKTAL_GRAPHICS_RESOURCES_LIGHT_H
#pragma once

#include <glm/glm.hpp>

namespace graphics::resources
{
	struct Light
	{
		glm::vec3 position;
		glm::vec3 color;
		float intensity;
		bool enabled;

		Light() : position(2.0f, 2.0f, 2.0f), color(1.0f, 1.0f, 1.0f), intensity(1.0f), enabled(true) {}
	};
}
#endif //VRAKTAL_GRAPHICS_RESOURCES_LIGHT_H
