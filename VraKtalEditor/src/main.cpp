#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <core/window.h>
#include <core/gpu/device.h>
#include <graphics/renderer.h>
#include <graphics/resources/scene.h>
#include <loaders/meshLoader.h>

#pragma comment(lib, "VraKtalEngine_Debug.lib")

int main()
{
	try
	{
		core::Window window(800, 600, "VraKtal Engine");
		core::gpu::Device device(window);
		graphics::Renderer renderer(window, device);
		loaders::MeshLoader loader;

		auto scene = std::make_shared<graphics::resources::Scene>();

		auto vikingRoomMesh = loader.LoadMesh("assets/models/viking_room.obj");

		auto vikingMaterial = std::make_shared<graphics::resources::Material>();
		vikingMaterial->name = "VikingRoomMaterial";
		vikingMaterial->albedo = glm::vec3(1.0f);
		vikingMaterial->metallic = 0.0f;
		vikingMaterial->roughness = 0.5f;
		vikingMaterial->useAlbedoTexture = true;
		vikingMaterial->albedoTexture = "assets/textures/viking_room.png";

		auto meshInstance1 = scene->AddMesh(vikingRoomMesh, vikingMaterial, glm::mat4(1.0));
		meshInstance1->SetPosition(glm::vec3(-1.5f, 0.5f, 0.0f));

		auto meshInstance2 = scene->AddMesh(vikingRoomMesh, vikingMaterial, glm::mat4(1.0f));
		meshInstance2->SetPosition(glm::vec3(1.5f, 0.5f, 0.0f));

		auto planeMesh = loaders::MeshLoader::CreatePlane(10.0f, 10.0f, 10, 10);

		auto planeMaterial = std::make_shared<graphics::resources::Material>();
		planeMaterial->name = "GroundMaterial";
		planeMaterial->albedo = glm::vec3(0.8f, 0.8f, 0.8f);
		planeMaterial->metallic = 0.0f;
		planeMaterial->roughness = 0.9f;
		planeMaterial->ao = 1.0f;

		auto planeInstance = scene->AddMesh(planeMesh, planeMaterial, glm::mat4(1.0f));

		graphics::resources::Light mainLight;
		mainLight.position = glm::vec3(3.0f, 4.0f, 3.0f);
		mainLight.color = glm::vec3(1.0f, 0.95f, 0.9f);
		mainLight.intensity = 15.0f;
		mainLight.enabled = true;
		mainLight.lightRadius = 0.001f;
		scene->AddLight(mainLight);

		renderer.SetScene(scene);

		renderer.EnableRayTracing();

		glm::vec3 cameraPos(4.0f, 3.0f, 4.0f);
		glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
		proj[1][1] *= -1;

		renderer.UpdateCamera(view, proj, cameraPos);

		float time = 0.0f;
		const float lightRadius = 1.0f;
		const float lightHeight = 1.0f;
		const float rotationSpeed = 0.01f;

		while (!window.ShouldClose())
		{
			window.PollEvents();

			time += 0.016f;

			float angle = time * rotationSpeed;

			scene->lights[0].position = glm::vec3(
				lightRadius * cos(angle),
				lightHeight,
				lightRadius * sin(angle)
			);

			scene->lights[0].intensity = 15.0f + 5.0f * sin(time * 2.0f);
			static int frameCounter = 0;

			renderer.DrawFrame();
		}

		renderer.Cleanup();
	}
	catch (const std::exception& e)
	{
		std::cerr << "ERROR: " << e.what() << std::endl;
		return -1;
	}

	return 0;
}