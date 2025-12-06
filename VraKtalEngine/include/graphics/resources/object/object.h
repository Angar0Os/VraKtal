#ifndef VRAKTAL_GRAPHICS_RESOURCES_OBJECT_OBJECT_H
#define VRAKTAL_GRAPHICS_RESOURCES_OBJECT_OBJECT_H
#pragma once

#include <graphics/resources/property/transform.h>
#include <graphics/renderer.h>

#include <memory>
#include <string>

namespace graphics::resources::object
{
	enum class ObjectType
	{
		Empty,
		StaticMesh,
		Camera,
		Light
	};

	class Object
	{
	public:
		std::string name;
		property::Transform transform;
		bool visible = true;

		Object() = default;
		explicit Object(const std::string& objectName) : name(objectName) {}
		virtual ~Object() = default;

		virtual ObjectType GetType() const { return ObjectType::Empty; }

		glm::vec3 GetPosition() const { return transform.position; }
		void SetPosition(const glm::vec3& pos) { transform.SetPosition(pos); }
		void Translate(const glm::vec3& offset) { transform.Translate(offset); }

		glm::mat4 GetTransformMatrix() const { return transform.GetMatrix(); }

		virtual void Update(float t, float dt) {}
		virtual void Render(graphics::Renderer& renderer) {}
	};
}

#endif //VRAKTAL_GRAPHICS_RESOURCES_OBJECT_OBJECT_H