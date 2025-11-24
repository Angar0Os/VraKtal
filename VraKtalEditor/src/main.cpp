#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

#include <core/window.h>
#include <core/gpu/device.h>
#include <core/gpu/buffer.h>
#include <core/gpu/accelerationStructure.h>
#include <core/gpu/commandBuffer.h>

#include <graphics/renderer.h>
#include <graphics/resources/scene.h>

#include <core/enum.h>

#include <loaders/meshLoader.h>

#pragma comment(lib, "VraKtalEngine_Debug.lib")

int main()
{
	try
	{
		core::Window window(800, 600, "VraKtal Engine");
		core::gpu::Device device(window);
		graphics::Renderer renderer(window, device);
		loaders::MeshLoader loaders;

		auto scene = std::make_shared<graphics::resources::Scene>();
		auto mesh = loaders.LoadMesh("assets/models/viking_room.obj");

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

		std::cout << "Creating acceleration structures..." << std::endl;

		std::unique_ptr<core::gpu::Buffer> vertexBuffer;
		std::unique_ptr<core::gpu::Buffer> indexBuffer;
		std::unique_ptr<core::gpu::AccelerationStructure> blas;
		std::unique_ptr<core::gpu::AccelerationStructure> tlas;

		{
			std::vector<float> vertices = {
				-0.5f, -0.5f, 0.0f,
				 0.5f, -0.5f, 0.0f,
				 0.0f,  0.5f, 0.0f
			};

			std::vector<uint32_t> indices = { 0, 1, 2 };

			core::gpu::BufferCreateInfo vertexBufferInfo{};
			vertexBufferInfo.size = vertices.size() * sizeof(float);
			vertexBufferInfo.usage = core::BufferUsage::VertexBuffer |
				core::BufferUsage::AccelerationStructureBuildInput |
				core::BufferUsage::ShaderDeviceAddress;
			vertexBufferInfo.memoryProperties = core::MemoryProperty::HostVisible |
				core::MemoryProperty::HostCoherent;

			vertexBuffer = std::make_unique<core::gpu::Buffer>(
				device.GetHandle(),
				device.GetPhysicalDevice(),
				vertexBufferInfo
			);
			vertexBuffer->CopyFrom(vertices.data(), vertices.size() * sizeof(float), 0);

			core::gpu::BufferCreateInfo indexBufferInfo{};
			indexBufferInfo.size = indices.size() * sizeof(uint32_t);
			indexBufferInfo.usage = core::BufferUsage::IndexBuffer |
				core::BufferUsage::AccelerationStructureBuildInput |
				core::BufferUsage::ShaderDeviceAddress;
			indexBufferInfo.memoryProperties = core::MemoryProperty::HostVisible |
				core::MemoryProperty::HostCoherent;

			indexBuffer = std::make_unique<core::gpu::Buffer>(
				device.GetHandle(),
				device.GetPhysicalDevice(),
				indexBufferInfo
			);
			indexBuffer->CopyFrom(indices.data(), indices.size() * sizeof(uint32_t), 0);

			std::cout << "Creating BLAS..." << std::endl;

			core::gpu::AccelerationStructureGeometry geometry{};
			geometry.vertexBuffer = vertexBuffer.get();
			geometry.vertexCount = static_cast<uint32_t>(vertices.size() / 3);
			geometry.vertexStride = sizeof(float) * 3;
			geometry.indexBuffer = indexBuffer.get();
			geometry.indexCount = static_cast<uint32_t>(indices.size());
			geometry.triangleCount = 1;
			geometry.opaque = true;

			core::gpu::AccelerationStructureCreateInfo blasInfo{};
			blasInfo.type = core::gpu::AccelerationStructureType::BottomLevel;
			blasInfo.geometries.push_back(geometry);
			blasInfo.preferFastTrace = true;
			blasInfo.allowUpdate = false;

			blas = std::make_unique<core::gpu::AccelerationStructure>(
				device.GetHandle(),
				device.GetPhysicalDevice(),
				blasInfo
			);

			std::cout << "BLAS created successfully!" << std::endl;
			std::cout << "BLAS device address: 0x" << std::hex << blas->GetDeviceAddress() << std::dec << std::endl;

			std::cout << "Creating TLAS..." << std::endl;

			float transform[3][4] = {
				{1.0f, 0.0f, 0.0f, 0.0f},
				{0.0f, 1.0f, 0.0f, 0.0f},
				{0.0f, 0.0f, 1.0f, 0.0f}
			};

			core::gpu::AccelerationStructureInstance instance{};
			instance.blas = blas.get();
			std::memcpy(&instance.transform, &transform, sizeof(transform));
			instance.instanceCustomIndex = 0;
			instance.mask = 0xFF;
			instance.instanceShaderBindingTableRecordOffset = 0;

			core::gpu::AccelerationStructureCreateInfo tlasInfo{};
			tlasInfo.type = core::gpu::AccelerationStructureType::TopLevel;
			tlasInfo.instances.push_back(instance);
			tlasInfo.preferFastTrace = true;
			tlasInfo.allowUpdate = false;

			tlas = std::make_unique<core::gpu::AccelerationStructure>(
				device.GetHandle(),
				device.GetPhysicalDevice(),
				tlasInfo
			);

			std::cout << "TLAS created successfully!" << std::endl;
			std::cout << "TLAS device address: 0x" << std::hex << tlas->GetDeviceAddress() << std::dec << std::endl;
		}

		std::cout << "Building acceleration structures..." << std::endl;

		core::gpu::CommandBufferCreateInfo cmdBufferInfo{};
		cmdBufferInfo.commandPool = device.GetCommandPool();
		cmdBufferInfo.count = 1;
		cmdBufferInfo.singleTime = true;
		cmdBufferInfo.level = core::CommandBufferLevel::Primary;

		core::gpu::CommandBuffer cmdBuffer(
			device.GetHandle(),
			device.GetGraphicsQueue(),
			cmdBufferInfo
		);

		cmdBuffer.Begin(0);
		cmdBuffer.BuildAccelerationStructure(blas.get());
		cmdBuffer.AccelerationStructureBarrier();
		cmdBuffer.BuildAccelerationStructure(tlas.get());
		cmdBuffer.End(0);
		cmdBuffer.SubmitAndWait();

		std::cout << "Acceleration structures built successfully!" << std::endl;

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