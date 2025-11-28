#ifndef VRAKTAL_GRAPHICS_RESOURCES_SCENE_H
#define VRAKTAL_GRAPHICS_RESOURCES_SCENE_H
#pragma once

#include <graphics/resources/object/object.h>
#include <graphics/resources/object/staticMesh.h>
#include <graphics/resources/object/camera.h>
#include <graphics/resources/object/mesh.h>
#include <graphics/resources/object/material.h>
#include <graphics/resources/object/light.h>

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
		std::vector<object::Light> lights;
		std::shared_ptr<object::Camera> activeCamera;

		Scene() = default;
		explicit Scene(const std::string& sceneName) : name(sceneName) {}

		object::Object* AddObject(std::shared_ptr<object::Object> obj)
		{
			objects.push_back(obj);
			return objects.back().get();
		}

		object::StaticMesh* AddStaticMesh(
			const std::string& objectName,
			std::shared_ptr<object::Mesh> mesh,
			std::shared_ptr<object::Material> material)
		{
			auto staticMesh = std::make_shared<object::StaticMesh>(objectName, mesh, material);
			objects.push_back(staticMesh);
			return staticMesh.get();
		}

		object::Camera* AddCamera(const std::string& cameraName)
		{
			auto camera = std::make_shared<object::Camera>(cameraName);
			objects.push_back(camera);

			if (!activeCamera)
				activeCamera = camera;

			return camera.get();
		}

		object::Object* FindObjectByName(const std::string& objectName)
		{
			auto it = std::find_if(objects.begin(), objects.end(),
				[&objectName](const auto& obj) { return obj->name == objectName; });
			return (it != objects.end()) ? it->get() : nullptr;
		}

		template<typename T>
		T* FindObjectByNameAs(const std::string& objectName)
		{
			auto obj = FindObjectByName(objectName);
			return obj ? dynamic_cast<T*>(obj) : nullptr;
		}

		std::vector<object::StaticMesh*> GetStaticMeshes()
		{
			std::vector<object::StaticMesh*> meshes;
			for (auto& obj : objects)
			{
				if (obj->GetType() == object::ObjectType::StaticMesh)
					meshes.push_back(static_cast<object::StaticMesh*>(obj.get()));
			}
			return meshes;
		}

		std::vector<object::Camera*> GetCameras()
		{
			std::vector<object::Camera*> cameras;
			for (auto& obj : objects)
			{
				if (obj->GetType() == object::ObjectType::Camera)
					cameras.push_back(static_cast<object::Camera*>(obj.get()));
			}
			return cameras;
		}

		void RemoveObject(const std::string& objectName)
		{
			auto it = std::remove_if(objects.begin(), objects.end(),
				[&objectName](const auto& obj) { return obj->name == objectName; });
			objects.erase(it, objects.end());
		}

		void RemoveObjectByIndex(size_t index)
		{
			if (index < objects.size())
				objects.erase(objects.begin() + index);
		}

		size_t GetObjectCount() const { return objects.size(); }

		size_t GetVisibleObjectCount() const
		{
			return std::count_if(objects.begin(), objects.end(),
				[](const auto& obj) { return obj->visible; });
		}

		void AddLight(const object::Light& light)
		{
			lights.push_back(light);
		}

		void RemoveLight(size_t index)
		{
			if (index < lights.size())
				lights.erase(lights.begin() + index);
		}

		object::Light* FindLightByName(const std::string& lightName)
		{
			auto it = std::find_if(lights.begin(), lights.end(),
				[&lightName](const object::Light& light) { return light.name == lightName; });
			return (it != lights.end()) ? &(*it) : nullptr;
		}

		size_t GetLightCount() const { return lights.size(); }

		void Clear()
		{
			objects.clear();
			lights.clear();
			activeCamera.reset();
		}

		void SetAllObjectsVisible(bool visible)
		{
			for (auto& obj : objects)
				obj->visible = visible;
		}
	};
}

#endif //VRAKTAL_GRAPHICS_RESOURCES_SCENE_H