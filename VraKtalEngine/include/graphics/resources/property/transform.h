#ifndef VRAKTAL_GRAPHICS_RESOURCES_TRANSFORM_H
#define VRAKTAL_GRAPHICS_RESOURCES_TRANSFORM_H
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace graphics::resources::property
{
	struct Transform
	{
		glm::vec3 position = glm::vec3(0.0f);
		glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 scale = glm::vec3(1.0f);

		Transform() = default;

		glm::mat4 GetMatrix() const
		{
			glm::mat4 mat = glm::mat4(1.0f);
			mat = glm::translate(mat, position);
			mat = mat * glm::mat4_cast(rotation);
			mat = glm::scale(mat, scale);
			return mat;
		}

		void SetPosition(const glm::vec3& pos) { position = pos; }
		void SetRotation(const glm::quat& rot) { rotation = rot; }
		void SetRotation(const glm::vec3& eulerAngles)
		{
			rotation = glm::quat(eulerAngles);
		}
		void SetScale(const glm::vec3& scl) { scale = scl; }

		void Translate(const glm::vec3& offset) { position += offset; }
		void Rotate(const glm::quat& rot) { rotation = rot * rotation; }
		void Rotate(float angle, const glm::vec3& axis)
		{
			rotation = glm::angleAxis(angle, axis) * rotation;
		}
		void Scale(const glm::vec3& factor) { scale *= factor; }

		glm::vec3 GetForward() const
		{
			return rotation * glm::vec3(0.0f, 0.0f, -1.0f);
		}

		glm::vec3 GetRight() const
		{
			return rotation * glm::vec3(1.0f, 0.0f, 0.0f);
		}

		glm::vec3 GetUp() const
		{
			return rotation * glm::vec3(0.0f, 1.0f, 0.0f);
		}
	};
}

#endif //VRAKTAL_GRAPHICS_RESOURCES_TRANSFORM_H