#include <graphics/renderer.h>

#include <memory>
#include <iostream>

using namespace graphics;

Renderer::Renderer(core::Window& window, core::gpu::Device& device)
	: m_window(window),
	m_device(device),
	m_currentFrame(0),
	m_running(true),
	m_frameCounter(0),
	m_viewMatrix(glm::mat4(1.0f)),
	m_projMatrix(glm::mat4(1.0f)),
	m_cameraPosition(glm::vec3(0.0f))
{
	CreateCommandBuffers();
}

Renderer::~Renderer()
{
	Cleanup();
}

void Renderer::CreateCommandBuffers()
{
	m_commandBuffers.clear();
	m_commandBuffers.reserve(core::gpu::Device::s_FRAMES_IN_FLIGHT);

	for (uint32_t i = 0; i < core::gpu::Device::s_FRAMES_IN_FLIGHT; i++)
	{
		core::gpu::SCommandBufferCreateInfo cmdInfo{};
		cmdInfo.device = &m_device;
		cmdInfo.level = core::ECommandBufferLevel::Primary;
		cmdInfo.count = 1;

		auto cmdBuffer = std::make_unique<core::gpu::CommandBuffer>(
			&m_device,
			cmdInfo
		);

		m_commandBuffers.push_back(std::move(cmdBuffer));
	}
}

void Renderer::PushObject(resources::object::StaticMesh& staticMesh, const glm::mat4& transform)
{
	if (!staticMesh.visible) return;
	if (!staticMesh.mesh || !staticMesh.material) return;

	if (m_meshBuffers.find(staticMesh.mesh.get()) == m_meshBuffers.end())
	{
		CreateMeshBuffers(staticMesh.mesh);
	}

	if (m_rayTracingEnabled && m_rtMeshData.find(staticMesh.mesh.get()) == m_rtMeshData.end())
	{
		CreateRTMeshBuffers(staticMesh.mesh);
		CreateBLAS(staticMesh.mesh.get());
	}

	m_staticMeshes.push_back({ &staticMesh, transform });
}

void Renderer::PushLight(const resources::object::Light& light)
{
	if (!light.enabled) return;

	LightData lightData;
	lightData.position = light.position;
	lightData.color = light.color;
	lightData.intensity = light.intensity;
	lightData.radius = light.radius;
	lightData.enabled = light.enabled;

	m_lights.push_back(lightData);
}

void Renderer::SetActiveCamera(resources::object::Camera& camera, const glm::mat4& transform)
{
	m_viewMatrix = camera.GetViewMatrix();
	m_projMatrix = camera.GetProjectionMatrix();
	m_cameraPosition = camera.GetPosition();
}

void Renderer::CreateMeshBuffers(std::shared_ptr<resources::object::Mesh> mesh)
{
	if (!mesh || mesh->vertices.empty() || mesh->indices.empty())
	{
		std::cerr << "ERROR: Invalid mesh data!" << std::endl;
		return;
	}

	MeshBuffers buffers;

	size_t vertexBufferSize = mesh->vertices.size() * sizeof(resources::object::Vertex);
	size_t indexBufferSize = mesh->indices.size() * sizeof(uint32_t);

	core::gpu::SBufferCreateInfo stagingVertexInfo{
		.size = vertexBufferSize,
		.usage = core::EBufferUsage::TransferSrc,
		.memoryProperties = core::EMemoryProperty::HostVisible | core::EMemoryProperty::HostCoherent
	};

	auto stagingVertexBuffer = std::make_unique<core::gpu::Buffer>(
		&m_device,
		stagingVertexInfo
	);

	core::gpu::SBufferCreateInfo stagingIndexInfo{
		.size = indexBufferSize,
		.usage = core::EBufferUsage::TransferSrc,
		.memoryProperties = core::EMemoryProperty::HostVisible | core::EMemoryProperty::HostCoherent
	};

	auto stagingIndexBuffer = std::make_unique<core::gpu::Buffer>(
		&m_device,
		stagingIndexInfo
	);

	stagingVertexBuffer->CopyFrom(mesh->vertices.data(), vertexBufferSize);
	stagingIndexBuffer->CopyFrom(mesh->indices.data(), indexBufferSize);

	core::gpu::SBufferCreateInfo vertexInfo{
		.size = vertexBufferSize,
		.usage = core::EBufferUsage::VertexBuffer | core::EBufferUsage::TransferDst,
		.memoryProperties = core::EMemoryProperty::DeviceLocal
	};

	buffers.vertexBuffer = std::make_unique<core::gpu::Buffer>(
		&m_device,
		vertexInfo
	);

	core::gpu::SBufferCreateInfo indexInfo{
		.size = indexBufferSize,
		.usage = core::EBufferUsage::IndexBuffer | core::EBufferUsage::TransferDst,
		.memoryProperties = core::EMemoryProperty::DeviceLocal
	};

	buffers.indexBuffer = std::make_unique<core::gpu::Buffer>(
		&m_device,
		indexInfo
	);

	buffers.indexCount = static_cast<uint32_t>(mesh->indices.size());

	core::gpu::SCommandBufferCreateInfo cmdInfo{};
	cmdInfo.device = &m_device;
	cmdInfo.level = core::ECommandBufferLevel::Primary;
	cmdInfo.count = 1;
	cmdInfo.singleTime = true;

	auto transferCmd = std::make_unique<core::gpu::CommandBuffer>(
		&m_device,
		cmdInfo
	);

	transferCmd->Begin(0);
	transferCmd->CopyBuffer(
		stagingVertexBuffer.get(),
		buffers.vertexBuffer.get(),
		vertexBufferSize
	);

	transferCmd->CopyBuffer(
		stagingIndexBuffer.get(),
		buffers.indexBuffer.get(),
		indexBufferSize
	);
	transferCmd->End(0);

	transferCmd->SubmitAndWait(&m_device);

	m_meshBuffers[mesh.get()] = std::move(buffers);
}

void Renderer::UpdateUniformBuffer(uint32_t frameIndex, const std::shared_ptr<resources::object::Material>& material)
{
	core::gpu::UniformBufferObject ubo{};

	ubo.view = m_viewMatrix;
	ubo.proj = m_projMatrix;
	ubo.viewPos = m_cameraPosition;

	ubo.numLights = std::min(static_cast<int>(m_lights.size()), core::gpu::MAX_LIGHTS);

	for (int i = 0; i < ubo.numLights; i++)
	{
		const auto& light = m_lights[i];
		ubo.lights[i].position = light.position;
		ubo.lights[i].color = light.color;
		ubo.lights[i].intensity = light.intensity;
		ubo.lights[i].enabled = light.enabled ? 1 : 0;
		ubo.lights[i].type = 0;
		ubo.lights[i].lightRadius = light.radius;
	}

	if (material)
	{
		ubo.albedo = material->albedo;
		ubo.metallic = material->metallic;
		ubo.roughness = material->roughness;
		ubo.ao = material->ao;
		ubo.emissive = material->emissive;

		ubo.useAlbedoMap = material->useAlbedoTexture ? 1 : 0;
		ubo.useNormalMap = material->useNormalTexture ? 1 : 0;
		ubo.useMetallicMap = material->useMetallicTexture ? 1 : 0;
		ubo.useRoughnessMap = material->useRoughnessTexture ? 1 : 0;
		ubo.useAOMap = material->useAOTexture ? 1 : 0;
		ubo.useEmissiveMap = material->useEmissiveTexture ? 1 : 0;
	}
	else
	{
		ubo.albedo = glm::vec3(1.0f);
		ubo.metallic = 0.0f;
		ubo.roughness = 0.5f;
		ubo.ao = 1.0f;
		ubo.emissive = glm::vec3(0.0f);

		ubo.useAlbedoMap = 0;
		ubo.useNormalMap = 0;
		ubo.useMetallicMap = 0;
		ubo.useRoughnessMap = 0;
		ubo.useAOMap = 0;
		ubo.useEmissiveMap = 0;
	}

	ubo.frameCount = static_cast<uint32_t>(m_frameCounter);

	auto* uniformBuffer = m_device.GetUniformBuffer(frameIndex);
	if (uniformBuffer)
	{
		uniformBuffer->CopyFrom(&ubo, sizeof(core::gpu::UniformBufferObject));
	}
}

void Renderer::DrawFrame(const core::gpu::Image* swapchainImage, uint32_t frameIndex, uint32_t imageIndex)
{
	if (!m_running) return;

	void* fence = m_device.GetInFlightFence(frameIndex);

	if (m_rayTracingEnabled && !m_staticMeshes.empty())
	{
		BuildTLAS();
		if (m_tlas)
		{
			for (uint32_t i = 0; i < core::gpu::Device::s_FRAMES_IN_FLIGHT; i++)
			{
				m_device.UpdateDescriptorWithTLAS(i, m_tlas.get());
			}
		}
	}

	auto& cmd = m_commandBuffers[frameIndex];

	cmd->Begin(0);

	const auto* colorImageHandle = m_device.GetColorImage();
	const auto* depthImageHandle = m_device.GetDepthImage();

	cmd->TransitionImageLayout(
		colorImageHandle,
		core::ImageLayout::Undefined,
		core::ImageLayout::ColorAttachment,
		false
	);

	cmd->TransitionImageLayout(
		swapchainImage,
		core::ImageLayout::Undefined,
		core::ImageLayout::TransferDst,
		false
	);

	cmd->TransitionImageLayout(
		depthImageHandle,
		core::ImageLayout::Undefined,
		core::ImageLayout::DepthStencilAttachment,
		true
	);

	cmd->BeginRendering(
		&m_device,
		colorImageHandle,
		depthImageHandle
	);

	cmd->BindPipeline(m_device.GetGraphicsPipeline());
	cmd->SetViewport(0.0f, 0.0f, &m_device);
	cmd->SetScissor(0, 0, &m_device);

	const resources::object::Material* lastMaterial = nullptr;

	for (const auto& renderable : m_staticMeshes)
	{
		auto* staticMesh = renderable.staticMesh;
		auto& mesh = staticMesh->mesh;
		auto& material = staticMesh->material;

		if (material.get() != lastMaterial)
		{
			UpdateUniformBuffer(frameIndex, material);

			cmd->BindDescriptorSets(
				&m_device,
				frameIndex,
				0
			);
			lastMaterial = material.get();
		}

		PushConstants pushConstants;
		pushConstants.model = renderable.transform;

		cmd->PushConstants(
			m_device.GetGraphicsPipeline(),
			static_cast<uint32_t>(core::ShaderStageFlags::Vertex),
			0,
			sizeof(PushConstants),
			&pushConstants
		);

		auto it = m_meshBuffers.find(mesh.get());
		if (it != m_meshBuffers.end())
		{
			const auto& buffers = it->second;
			cmd->BindVertexBuffer(buffers.vertexBuffer.get());
			cmd->BindIndexBuffer(buffers.indexBuffer.get());
			cmd->DrawIndexed(buffers.indexCount);
		}
	}

	cmd->EndRendering();

	cmd->TransitionImageLayout(
		colorImageHandle,
		core::ImageLayout::ColorAttachment,
		core::ImageLayout::TransferSrc,
		false
	);

	cmd->ResolveImage(
		colorImageHandle,
		swapchainImage,
		&m_device
	);

	cmd->TransitionImageLayout(
		swapchainImage,
		core::ImageLayout::TransferDst,
		core::ImageLayout::Present,
		false
	);

	cmd->End(0);

	void* waitSemaphore = m_device.GetImageAvailableSemaphore(frameIndex);
	void* signalSemaphore = m_device.GetRenderFinishedSemaphore(frameIndex);

	m_commandBuffers[frameIndex]->Submit(&m_device, waitSemaphore, signalSemaphore, fence);

	m_frameCounter++;

	m_staticMeshes.clear();
	m_lights.clear();
}

void Renderer::Cleanup()
{
	m_device.WaitIdle();

	if (!m_running) return;
	m_running = false;
	m_meshBuffers.clear();
	m_commandBuffers.clear();
	m_staticMeshes.clear();
	m_lights.clear();
	m_device.Cleanup();
}

void Renderer::DisableRayTracing()
{
	if (!m_rayTracingEnabled) return;

	m_device.WaitIdle();
	m_tlas.reset();
	m_rtMeshData.clear();
	m_rayTracingEnabled = false;

	std::cout << "Ray tracing disabled." << std::endl;
}

void Renderer::CreateRTMeshBuffers(std::shared_ptr<resources::object::Mesh> mesh)
{
	if (!mesh || mesh->vertices.empty() || mesh->indices.empty())
	{
		std::cerr << "ERROR: Invalid mesh data for RT buffers!" << std::endl;
		return;
	}

	RTMeshData rtData;

	size_t vertexBufferSize = mesh->vertices.size() * sizeof(resources::object::Vertex);
	size_t indexBufferSize = mesh->indices.size() * sizeof(uint32_t);

	core::gpu::SBufferCreateInfo rtVertexInfo{
		.size = vertexBufferSize,
		.usage = core::EBufferUsage::AccelerationStructureBuildInput |
				 core::EBufferUsage::ShaderDeviceAddress |
				 core::EBufferUsage::StorageBuffer,
		.memoryProperties = core::EMemoryProperty::HostVisible | core::EMemoryProperty::HostCoherent
	};

	rtData.rtVertexBuffer = std::make_unique<core::gpu::Buffer>(
		&m_device,
		rtVertexInfo
	);
	rtData.rtVertexBuffer->CopyFrom(mesh->vertices.data(), vertexBufferSize);

	core::gpu::SBufferCreateInfo rtIndexInfo{
		.size = indexBufferSize,
		.usage = core::EBufferUsage::AccelerationStructureBuildInput |
				 core::EBufferUsage::ShaderDeviceAddress |
				 core::EBufferUsage::StorageBuffer,
		.memoryProperties = core::EMemoryProperty::HostVisible | core::EMemoryProperty::HostCoherent
	};

	rtData.rtIndexBuffer = std::make_unique<core::gpu::Buffer>(
		&m_device,
		rtIndexInfo
	);
	rtData.rtIndexBuffer->CopyFrom(mesh->indices.data(), indexBufferSize);

	m_rtMeshData[mesh.get()] = std::move(rtData);
}

void Renderer::CreateBLAS(resources::object::Mesh* mesh)
{
	auto it = m_rtMeshData.find(mesh);
	if (it == m_rtMeshData.end())
	{
		std::cerr << "ERROR: RT mesh data not found for BLAS creation!" << std::endl;
		return;
	}

	auto& rtData = it->second;
	core::gpu::SAccelerationStructureGeometry geometry{};
	geometry.vertexBuffer = rtData.rtVertexBuffer.get();
	geometry.vertexCount = static_cast<uint32_t>(mesh->vertices.size());
	geometry.vertexStride = sizeof(resources::object::Vertex);
	geometry.indexBuffer = rtData.rtIndexBuffer.get();
	geometry.indexCount = static_cast<uint32_t>(mesh->indices.size());
	geometry.triangleCount = geometry.indexCount / 3;
	geometry.opaque = true;

	core::gpu::SAccelerationStructureCreateInfo blasInfo{};
	blasInfo.type = core::gpu::EAccelerationStructureType::BottomLevel;
	blasInfo.geometries.push_back(geometry);
	blasInfo.preferFastTrace = true;
	blasInfo.allowUpdate = false;

	rtData.blas = std::make_unique<core::gpu::AccelerationStructure>(
		&m_device,
		blasInfo
	);
}

void Renderer::EnableRayTracing()
{
	if (m_rayTracingEnabled) return;

	m_rayTracingEnabled = true;
	std::cout << "Ray tracing enabled." << std::endl;
}

void Renderer::BuildTLAS()
{
	if (m_staticMeshes.empty())
	{
		return;
	}

	std::vector<core::gpu::SAccelerationStructureInstance> instances;
	instances.reserve(m_staticMeshes.size());

	uint32_t instanceIndex = 0;

	for (const auto& renderable : m_staticMeshes)
	{
		auto* mesh = renderable.staticMesh->mesh.get();

		auto it = m_rtMeshData.find(mesh);
		if (it == m_rtMeshData.end() || !it->second.blas)
		{
			std::cerr << "Warning: BLAS not found for mesh instance!" << std::endl;
			continue;
		}

		const glm::mat4 mat = renderable.transform;

		float transform[3][4] = {
			{mat[0][0], mat[1][0], mat[2][0], mat[3][0]},
			{mat[0][1], mat[1][1], mat[2][1], mat[3][1]},
			{mat[0][2], mat[1][2], mat[2][2], mat[3][2]}
		};

		core::gpu::SAccelerationStructureInstance instance{};
		std::memcpy(&instance.transform, &transform, sizeof(transform));
		instance.instanceCustomIndex = instanceIndex++;
		instance.mask = 0xFF;
		instance.instanceShaderBindingTableRecordOffset = 0;
		instance.blas = it->second.blas.get();

		instances.push_back(instance);
	}

	if (instances.empty())
	{
		return;
	}

	core::gpu::SAccelerationStructureCreateInfo tlasInfo{};
	tlasInfo.type = core::gpu::EAccelerationStructureType::TopLevel;
	tlasInfo.instances = instances;
	tlasInfo.preferFastTrace = true;
	tlasInfo.allowUpdate = false;

	m_tlas = std::make_unique<core::gpu::AccelerationStructure>(
		&m_device,
		tlasInfo
	);

	RebuildAccelerationStructures();
}

void Renderer::RebuildAccelerationStructures()
{
	core::gpu::SCommandBufferCreateInfo cmdInfo{};
	cmdInfo.device = &m_device;
	cmdInfo.count = 1;
	cmdInfo.singleTime = true;
	cmdInfo.level = core::ECommandBufferLevel::Primary;

	core::gpu::CommandBuffer cmdBuffer(
		&m_device,
		cmdInfo
	);

	cmdBuffer.Begin(0);

	for (auto& [mesh, rtData] : m_rtMeshData)
	{
		if (rtData.blas)
		{
			cmdBuffer.BuildAccelerationStructure(rtData.blas.get());
			cmdBuffer.AccelerationStructureBarrier();
		}
	}

	if (m_tlas)
	{
		cmdBuffer.BuildAccelerationStructure(m_tlas.get());
	}

	cmdBuffer.End(0);
	cmdBuffer.SubmitAndWait(&m_device);

	std::cout << "Acceleration structures built successfully!" << std::endl;
}