#ifndef VRAKTAL_GRAPHICS_RESOURCES_OBJECT_CAMERA_H
#define VRAKTAL_GRAPHICS_RESOURCES_OBJECT_CAMERA_H
#pragma once

#include <graphics/resources/object/object.h>
#include <graphics/renderer.h>
#include <glm/gtc/matrix_transform.hpp>

namespace graphics::resources::object
{
	class Camera : public Object
	{
	public:
		float fov = 45.0f;
		float zNear = 0.1f;
		float zFar = 100.0f;
		float aspectRatio = 16.0f / 9.0f;

		Camera() {}

		explicit Camera(const std::string& name) : Object(name)
		{
		}

		void Render(graphics::Renderer& renderer) override;
		void Update(float t, float dt) override;


		glm::mat4 GetViewMatrix() const
		{
			glm::vec3 pos = transform.position;
			glm::vec3 forward = transform.GetForward();
			glm::vec3 up = transform.GetUp();
			return glm::lookAtLH(pos, pos + forward, up);
		}

		glm::mat4 GetProjectionMatrix() const
		{
			glm::mat4 proj = glm::perspectiveLH(
				glm::radians(fov),
				aspectRatio,
				zNear,
				zFar
			);

			proj[1][1] *= -1.0f;

			return proj;
		}

		ObjectType GetType() const override { return ObjectType::Camera; }

		void LookAt(const glm::vec3& target)
		{
			glm::vec3 direction = glm::normalize(target - transform.position);
			glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

			glm::vec3 right = glm::normalize(glm::cross(direction, up));
			up = glm::cross(right, direction);

			glm::mat3 rotMatrix;
			rotMatrix[0] = right;
			rotMatrix[1] = up;
			rotMatrix[2] = -direction;

			transform.rotation = glm::quat_cast(rotMatrix);
		}
	};
}

#endif //VRAKTAL_GRAPHICS_RESOURCES_OBJECT_CAMERA_H