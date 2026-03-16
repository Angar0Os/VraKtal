#include <iostream>
#include <core/window.h>
#include <core/gpu/device.h>
#include <core/gpu/image.h>
#include <core/gpu/imguiContext.h>
#include <graphics/renderer.h>
#include <graphics/renderPass/gBufferPass.h>
#include <graphics/resources/object/light.h>
#include <graphics/resources/object/material.h>
#include <loaders/meshLoader.h>
#include <loaders/materialLoader.h>
#include "imGuiWindows.h"
#include "utils/yamlParser.h"

#pragma comment(lib, "VraKtalEngine_Debug.lib")

int main()
{
	core::Window window(800, 600, "VraKtal Engine");
	core::gpu::Device device(window);
	graphics::Renderer renderer(window, device);
	loaders::MeshLoader loader(&device);
	ImGuiWindows imGuiWindows = ImGuiWindows(&renderer);

	device.GetImGuiContext()->BindPrepareDrawData([&]()
		{
			imGuiWindows.PrepareImGuiWindows();
		});

	auto vikingRoomMesh = loader.LoadMesh("assets/models/viking_room.obj");
	auto planeMesh = loader.CreatePlane(10.0f, 10.0f, 10, 10);

	auto* matLayout = renderer.GetPass<graphics::GBufferPass>("GBuffer")->GetMaterialLayout();

	{
		graphics::resources::object::Material mat;
		mat.SetTexture("assets/textures/viking_room.png", "albedo");
		mat.SetMetallicRoughness(0.0f, 0.8f);
		vikingRoomMesh->materials.push_back(
			loaders::MaterialLoader::Load(device, mat, matLayout)
		);
	}

	{
		graphics::resources::object::Material mat;
		mat.SetAlbedo(0.9f, 0.0f, 0.2f);
		mat.SetMetallicRoughness(0.0f, 0.9f);
		planeMesh->materials.push_back(
			loaders::MaterialLoader::Load(device, mat, matLayout)
		);
	}

	const float aspectRatio = 800.0f / 600.0f;
	glm::mat4 projection = glm::perspectiveLH_ZO(
		glm::radians(45.0f),
		aspectRatio,
		0.1f,
		100.0f
	);
	projection[1][1] *= -1;

	glm::vec3 cameraPosition = glm::vec3(0.0f, 3.0f, -5.0f);
	glm::mat4 view = glm::lookAtLH(
		cameraPosition,
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	float       time = 0.0f;
	const float timeStep = (1.0f / 240.0f) / 5.0f;

	uint32_t currentFrameIndex = 0;
	uint32_t frameCounter = 0;

	utils::YamlParser parser("project.yaml");
	std::vector<graphics::resources::Light> lights;
	if (parser.IsValid())
		lights = parser.LoadLights();

	while (!window.ShouldClose())
	{
		window.PollEvents();

		uint32_t imageIndex = device.AcquireNextImage(currentFrameIndex);
		if (imageIndex == UINT32_MAX)
		{
			device.RecreateSwapchain();
			continue;
		}

		time += timeStep;

		renderer.SetCamera(view, projection);

		renderer.PushMesh(planeMesh.get(), glm::mat4(1.0f));

		glm::mat4 meshTransform1 = glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, 0.1f, 0.0f));
		meshTransform1 = glm::rotate(meshTransform1, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		renderer.PushMesh(vikingRoomMesh.get(), meshTransform1);

		glm::mat4 meshTransform2 = glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 0.1f, 0.0f));
		meshTransform2 = glm::rotate(meshTransform2, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		renderer.PushMesh(vikingRoomMesh.get(), meshTransform2);

		graphics::resources::Light light1;
		light1.name = "Yellow Light 1";
		light1.position = glm::vec3(3.0f * glm::cos(time), 4.0f, 3.0f * glm::sin(time));
		light1.color = glm::vec3(1.0f, 0.9f, 0.2f);
		light1.intensity = 10.0f;
		light1.radius = 0.1f;
		light1.enabled = true;
		renderer.PushLight(light1);

		graphics::resources::Light light2;
		light2.name = "Yellow Light 2";
		light2.position = glm::vec3(3.0f * glm::cos(time + glm::pi<float>()), 3.0f, 3.0f * glm::sin(time + glm::pi<float>()));
		light2.color = glm::vec3(1.0f, 0.85f, 0.1f);
		light2.intensity = 8.0f;
		light2.radius = 0.1f;
		light2.enabled = true;
		renderer.PushLight(light2);

		graphics::resources::Light light3;
		light3.name = "Blue Light";
		light3.position = glm::vec3(0.0f, 6.0f, 0.0f);
		light3.color = glm::vec3(0.2f, 0.4f, 1.0f);
		light3.intensity = 15.0f;
		light3.radius = 0.1f;
		light3.enabled = true;
		renderer.PushLight(light3);

		renderer.Render(imageIndex);
		device.Present(imageIndex, currentFrameIndex);

		currentFrameIndex = (currentFrameIndex + 1) % core::gpu::Device::s_FRAMES_IN_FLIGHT;
		frameCounter++;
	}

	device.WaitIdle();
	renderer.Cleanup();

	return 0;
}