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
		auto mesh = loader.LoadMesh("assets/models/viking_room.obj");

		auto material = std::make_shared<graphics::resources::Material>();
		material->name = "VikingRoomMaterial";
		material->albedo = glm::vec3(1.0f);
		material->metallic = 0.0f;
		material->roughness = 0.5f;
		material->useAlbedoTexture = true;
		material->albedoTexture = "assets/textures/viking_room.png";

		auto meshInstance = scene->AddMesh(mesh, material, glm::mat4(1.0));
		meshInstance->SetPosition(glm::vec3(-1.0f, 0.0f, 0.0f));

		auto cube2 = scene->AddMesh(mesh, material, glm::mat4(1.0f));
		cube2->SetPosition(glm::vec3(1.0f, 0.0f, 0.0f));

		graphics::resources::Light light1;
		light1.position = glm::vec3(2.0f, 2.0f, 2.0f);
		light1.color = glm::vec3(1.0f, 1.0f, 1.0f);
		light1.intensity = 1.0f;
		light1.enabled = true;
		scene->AddLight(light1);

		renderer.SetScene(scene);

		renderer.EnableRayTracing();

		glm::vec3 cameraPos(2.0f, 2.0f, 2.0f);
		glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 10.0f);
		proj[1][1] *= -1;

		renderer.UpdateCamera(view, proj, cameraPos);

		std::cout << "Entering render loop..." << std::endl;

		while (!window.ShouldClose())
		{
			window.PollEvents();
			renderer.DrawFrame();
		}

		renderer.Cleanup();

		std::cout << "Application terminated successfully!" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "ERROR: " << e.what() << std::endl;
		return -1;
	}

	return 0;
}