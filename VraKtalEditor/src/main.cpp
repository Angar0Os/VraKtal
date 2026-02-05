#include <iostream>

#include <core/window.h>
#include <core/gpu/device.h>
#include <core/gpu/image.h>

#include <graphics/renderer.h>
#include <graphics/resources/object/light.h>

#include <core/gpu/imguiContext.h>
#include "imGuiWindows.h"

#include <loaders/meshLoader.h>

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

	float time = 0.0f;
	float timeStepT = 1.0f / 240.0f;
	const float timeStep = timeStepT / 5.0f;

	uint32_t currentFrameIndex = 0;
	uint32_t frameCounter = 0;

	utils::YamlParser parser("project.yaml");

	std::vector<graphics::resources::Light> lights;

	if (parser.IsValid()) {
		lights = parser.LoadLights();
	}

	while (!window.ShouldClose())
	{
		window.PollEvents();

		uint32_t imageIndex = device.AcquireNextImage(currentFrameIndex);
		if (imageIndex == UINT32_MAX)
		{
			device.RecreateSwapchain();
			continue;
		}

		const core::gpu::Image* swapchainImage = device.GetSwapchainImage(imageIndex);
		if (!swapchainImage)
			continue;

		time += timeStep;

		renderer.SetCamera(view, projection);

		glm::mat4 planeTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		renderer.PushMesh(planeMesh.get(), planeTransform);

		glm::mat4 meshTransform1 = glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, 0.1f, 0.0f));
		meshTransform1 = glm::rotate(meshTransform1, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		renderer.PushMesh(vikingRoomMesh.get(), meshTransform1);

		glm::mat4 meshTransform2 = glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 0.1f, 0.0f));
		meshTransform2 = glm::rotate(meshTransform2, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		renderer.PushMesh(vikingRoomMesh.get(), meshTransform2);

		graphics::resources::Light mainLight;
		mainLight.name = "Main Light";
		mainLight.position = glm::vec3(
			3.0f * glm::cos(time),
			4.0f,
			3.0f * glm::sin(time)
		);
		mainLight.color = glm::vec3(1.0f, 0.95f, 0.4f);
		mainLight.intensity = 1.0f;
		mainLight.radius = 0.2f + 0.2f * glm::sin(time * 2.0f);
		mainLight.enabled = true;

		renderer.PushLight(mainLight);

		renderer.Render(swapchainImage, imageIndex);
		device.Present(imageIndex);

		currentFrameIndex = (currentFrameIndex + 1) % core::gpu::Device::s_FRAMES_IN_FLIGHT;
		frameCounter++;
	}

	device.WaitIdle();
	renderer.Cleanup();

	return 0;
}