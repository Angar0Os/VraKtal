#include <iostream>

#include <core/window.h>
#include <core/gpu/device.h>
#include <core/gpu/image.h>

#include <graphics/renderer.h>
#include <graphics/resources/scene.h>
#include <graphics/resources/object/material.h>
#include <graphics/resources/object/camera.h>

#include <loaders/meshLoader.h>

#pragma comment(lib, "VraKtalEngine_Debug.lib")

int main()
{
	core::Window window(800, 600, "VraKtal Engine");
	core::gpu::Device device(window);
	graphics::Renderer renderer(window, device);
	loaders::MeshLoader loader;

	auto scene = std::make_shared<graphics::resources::Scene>("MainScene");

	auto camera = scene->AddCamera("MainCamera");
	camera->SetPosition(glm::vec3(0.0f, 1.5f, 5.0f));
	camera->LookAt(glm::vec3(0.0f, 0.5f, 0.0f));
	camera->fov = 45.0f;
	camera->aspectRatio = 800.0f / 600.0f;
	camera->zNear = 0.1f;
	camera->zFar = 100.0f;

	auto vikingRoomMesh = loader.LoadMesh("assets/models/viking_room.obj");

	auto vikingMaterial = std::make_shared<graphics::resources::object::Material>();
	vikingMaterial->name = "VikingRoomMaterial";
	vikingMaterial->albedo = glm::vec3(1.0f);
	vikingMaterial->metallic = 0.0f;
	vikingMaterial->roughness = 0.04f;
	vikingMaterial->useAlbedoTexture = true;
	vikingMaterial->albedoTexture = "assets/textures/viking_room.png";
    vikingMaterial->emissive = glm::vec3(0.0f);

	auto meshObj1 = scene->AddStaticMesh("VikingRoom1", vikingRoomMesh, vikingMaterial);
	meshObj1->SetPosition(glm::vec3(-1.5f, 0.5f, 0.0f));

	auto meshObj2 = scene->AddStaticMesh("VikingRoom2", vikingRoomMesh, vikingMaterial);
	meshObj2->SetPosition(glm::vec3(1.5f, 0.5f, 0.0f));

	auto planeMesh = loaders::MeshLoader::CreatePlane(10.0f, 10.0f, 10, 10);

	auto planeMaterial = std::make_shared<graphics::resources::object::Material>();
	planeMaterial->name = "GroundMaterial";
	planeMaterial->albedo = glm::vec3(0.8f, 0.8f, 0.8f);
	planeMaterial->metallic = 0.0f;
	planeMaterial->roughness = 0.04f;
	planeMaterial->ao = 1.0f;
	planeMaterial->emissive = glm::vec3(0.0f);

	auto planeObj = scene->AddStaticMesh("Ground", planeMesh, planeMaterial);

	renderer.SetScene(scene);
	renderer.EnableRayTracing();

    float t = 0.0f;
	
	graphics::resources::object::Light mainLight;
	mainLight.position = glm::vec3(3.0f * glm::cos(t), 4.0f, 3.0f * glm::sin(t));
	mainLight.color = glm::vec3(1.0f, 0.0f, 0.0f);
	mainLight.intensity = 5.0f;
	mainLight.enabled = true;
	mainLight.radius = 0.2f + 0.2f * glm::sin(t * 2.0f);
	auto light = scene->AddLight(mainLight);

    uint32_t frameCounter = 0;

	while (!window.ShouldClose())
	{
		window.PollEvents();
		renderer.DrawFrame();
	}

	renderer.Cleanup();

	return 0;
}