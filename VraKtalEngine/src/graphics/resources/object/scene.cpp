#include <graphics/resources/scene.h>

using namespace graphics::resources;

object::Object* Scene::AddObject(std::shared_ptr<object::Object> obj)
{
	objects.push_back(obj);
	return objects.back().get();
}

object::StaticMesh* Scene::AddStaticMesh(
	const std::string& objectName,
	std::shared_ptr<object::Mesh> mesh,
	std::shared_ptr<object::Material> material)
{
	auto staticMesh = std::make_shared<object::StaticMesh>(objectName, mesh, material);
	objects.push_back(staticMesh);
	return staticMesh.get();
}

object::Camera* Scene::AddCamera(const std::string& cameraName)
{
	auto camera = std::make_shared<object::Camera>(cameraName);
	objects.push_back(camera);

	if (!activeCamera)
		activeCamera = camera;

	return camera.get();
}

object::Object* Scene::FindObjectByName(const std::string& objectName)
{
	auto it = std::find_if(objects.begin(), objects.end(),
		[&objectName](const auto& obj) { return obj->name == objectName; });
	return (it != objects.end()) ? it->get() : nullptr;
}

std::vector<object::StaticMesh*> Scene::GetStaticMeshes()
{
	std::vector<object::StaticMesh*> meshes;
	for (auto& obj : objects)
	{
		if (obj->GetType() == object::ObjectType::StaticMesh)
			meshes.push_back(static_cast<object::StaticMesh*>(obj.get()));
	}
	return meshes;
}

std::vector<object::Camera*> Scene::GetCameras()
{
	std::vector<object::Camera*> cameras;
	for (auto& obj : objects)
	{
		if (obj->GetType() == object::ObjectType::Camera)
			cameras.push_back(static_cast<object::Camera*>(obj.get()));
	}
	return cameras;
}

void Scene::RemoveObject(const std::string& objectName)
{
	auto it = std::remove_if(objects.begin(), objects.end(),
		[&objectName](const auto& obj) { return obj->name == objectName; });
	objects.erase(it, objects.end());
}

void Scene::RemoveObjectByIndex(size_t index)
{
	if (index < objects.size())
	{
		objects.erase(objects.begin() + index);
	}
}

size_t Scene::GetObjectCount() const
{
	return objects.size();
}

size_t Scene::GetVisibleObjectCount() const
{
	return std::count_if(objects.begin(), objects.end(),
		[](const auto& obj) { return obj->visible; });
}

object::Light* Scene::AddLight(const object::Light& light)
{
	lights.push_back(light);
	return &lights.back();
}

void Scene::RemoveLight(size_t index)
{
	if (index < lights.size())
	{
		lights.erase(lights.begin() + index);
	}
}

object::Light* Scene::FindLightByName(const std::string& lightName)
{
	auto it = std::find_if(lights.begin(), lights.end(),
		[&lightName](const object::Light& light) { return light.name == lightName; });
	return (it != lights.end()) ? &(*it) : nullptr;
}

size_t Scene::GetLightCount() const
{
	return lights.size();
}

void Scene::Clear()
{
	objects.clear();
	lights.clear();
	activeCamera.reset();
}

void Scene::SetAllObjectsVisible(bool visible)
{
	for (auto& obj : objects)
	{
		obj->visible = visible;
	}
}