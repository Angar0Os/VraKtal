#ifndef VRAKTAL_GRAPHICS_RESOURCES_SCENE_H
#define VRAKTAL_GRAPHICS_RESOURCES_SCENE_H
#pragma once

#include <graphics/resources/object/object.h>
#include <graphics/resources/object/staticMesh.h>
#include <graphics/resources/object/camera.h>
#include <graphics/resources/object/mesh.h>
#include <graphics/resources/object/material.h>
#include <graphics/resources/object/light.h>

#include <core/gpu/buffer.h>

#include <memory>
#include <vector>
#include <string>
#include <algorithm>

namespace graphics::resources
{
	class Scene
	{
	public:
		std::string name;
		std::vector<std::shared_ptr<object::Object>> objects;
		std::vector<Light> lights;
		std::shared_ptr<object::Camera> activeCamera;

		Scene() = default;
		explicit Scene(const std::string& sceneName) : name(sceneName) {}

		object::Object* AddObject(std::shared_ptr<object::Object> obj);

		object::StaticMesh* AddStaticMesh(
			const std::string& objectName,
			std::shared_ptr<Mesh> mesh,
			std::shared_ptr<object::Material> material);

		object::Camera* AddCamera(const std::string& cameraName);

		object::Object* FindObjectByName(const std::string& objectName);

		std::vector<object::StaticMesh*> GetStaticMeshes();

		std::vector<object::Camera*> GetCameras();

		void RemoveObject(const std::string& objectName);

		void RemoveObjectByIndex(size_t index);

		size_t GetObjectCount() const;

		size_t GetVisibleObjectCount() const;

		Light* AddLight(const Light& light);

		void RemoveLight(size_t index);

		Light* FindLightByName(const std::string& lightName);

		size_t GetLightCount() const;

		void Clear();

		void SetAllObjectsVisible(bool visible);
	};
}

#endif //VRAKTAL_GRAPHICS_RESOURCES_SCENE_H