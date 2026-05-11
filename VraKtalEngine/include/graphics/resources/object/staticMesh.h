#ifndef VRAKTAL_GRAPHICS_RESOURCES_OBJECT_STATICMESH_H
#define VRAKTAL_GRAPHICS_RESOURCES_OBJECT_STATICMESH_H
#pragma once

#include <graphics/resources/object/object.h>
#include <graphics/resources/object/mesh.h>
#include <graphics/assets/material.h>
#include <memory>

namespace graphics::resources::object
{
	class StaticMesh : public Object
	{
	public:
		std::shared_ptr<Mesh> mesh;
		std::shared_ptr<graphics::assets::Material> material;

		StaticMesh() {}

		StaticMesh(const std::string& name,
			std::shared_ptr<Mesh> meshPtr,
			std::shared_ptr<graphics::assets::Material> matPtr)
			: Object(name), mesh(meshPtr), material(matPtr)
		{
		}

		ObjectType GetType() const override { return ObjectType::StaticMesh; }
	};
}

#endif //VRAKTAL_GRAPHICS_RESOURCES_OBJECT_STATICMESH_H