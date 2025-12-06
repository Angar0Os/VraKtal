#ifndef VRAKTAL_GRAPHICS_RESOURCES_LIGHT_H
#define VRAKTAL_GRAPHICS_RESOURCES_LIGHT_H
#pragma once

#include <glm/glm.hpp>
#include <string>

#include <graphics/renderer.h>

namespace graphics::resources::object
{
	enum class LightType
	{
		Point = 0,
		Directional = 1,
		Spot = 2
	};

	struct Light
	{
		std::string name;
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

		Light()
			: name("Light"),
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
		{
		}

		static Light CreatePointLight(const glm::vec3& position, const glm::vec3& color, 
									  float intensity, const std::string& name = "PointLight")
		{
			Light light;
			light.name = name;
			light.position = position;
			light.color = color;
			light.intensity = intensity;
			light.type = LightType::Point;
			light.lightRadius = 0.5f;
			return light;
		}

		static Light CreateDirectionalLight(const glm::vec3& direction, const glm::vec3& color, 
											float intensity, const std::string& name = "DirectionalLight")
		{
			Light light;
			light.name = name;
			light.direction = glm::normalize(direction);
			light.color = color;
			light.intensity = intensity;
			light.type = LightType::Directional;
			light.lightRadius = 0.5f;
			return light;
		}

		static Light CreateSpotLight(const glm::vec3& position, const glm::vec3& direction,
			const glm::vec3& color, float intensity,
			float innerAngle, float outerAngle,
			const std::string& name = "SpotLight")
		{
			Light light;
			light.name = name;
			light.position = position;
			light.direction = glm::normalize(direction);
			light.color = color;
			light.intensity = intensity;
			light.type = LightType::Spot;
			light.innerConeAngle = innerAngle;
			light.outerConeAngle = outerAngle;
			light.lightRadius = 0.3f;
			return light;
		}

		void SetColor(float r, float g, float b)
		{
			color = glm::vec3(r, g, b);
		}

		void LookAt(const glm::vec3& target)
		{
			direction = glm::normalize(target - position);
		}

		void Translate(const glm::vec3& offset)
		{
			position += offset;
		}
	};
}
#endif //VRAKTAL_GRAPHICS_RESOURCES_LIGHT_H