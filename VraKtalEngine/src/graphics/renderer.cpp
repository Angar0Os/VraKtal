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
	m_commandBuffers.reserve(core::gpu::Device::FRAMES_IN_FLIGHT);

	for (uint32_t i = 0; i < core::gpu::Device::FRAMES_IN_FLIGHT; i++)
	{
		core::gpu::CommandBufferCreateInfo cmdInfo{};
		cmdInfo.commandPool = m_device.GetCommandPool();
		cmdInfo.level = core::CommandBufferLevel::Primary;
		cmdInfo.count = 1;

		auto cmdBuffer = std::make_unique<core::gpu::CommandBuffer>(
			m_device.GetHandle(),
			m_device.GetGraphicsQueue(),
			cmdInfo
		);

		m_commandBuffers.push_back(std::move(cmdBuffer));
	}
}

void Renderer::SetScene(std::shared_ptr<resources::Scene> scene)
{
	m_scene = scene;

	if (m_scene)
	{
		auto staticMeshes = m_scene->GetStaticMeshes();
		for (auto* staticMesh : staticMeshes)
		{
			if (staticMesh->mesh &&
				m_meshBuffers.find(staticMesh->mesh.get()) == m_meshBuffers.end())
			{
				CreateMeshBuffers(staticMesh->mesh);
			}
		}

		if (m_rayTracingEnabled)
		{
			DisableRayTracing();
			EnableRayTracing();
		}
	}
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

	core::gpu::BufferCreateInfo stagingVertexInfo{
		.size = vertexBufferSize,
		.usage = core::BufferUsage::TransferSrc,
		.memoryProperties = core::MemoryProperty::HostVisible | core::MemoryProperty::HostCoherent
	};

	auto stagingVertexBuffer = std::make_unique<core::gpu::Buffer>(
		m_device.GetHandle(),
		m_device.GetPhysicalDevice(),
		stagingVertexInfo
	);

	core::gpu::BufferCreateInfo stagingIndexInfo{
		.size = indexBufferSize,
		.usage = core::BufferUsage::TransferSrc,
		.memoryProperties = core::MemoryProperty::HostVisible | core::MemoryProperty::HostCoherent
	};

	auto stagingIndexBuffer = std::make_unique<core::gpu::Buffer>(
		m_device.GetHandle(),
		m_device.GetPhysicalDevice(),
		stagingIndexInfo
	);

	stagingVertexBuffer->CopyFrom(mesh->vertices.data(), vertexBufferSize);
	stagingIndexBuffer->CopyFrom(mesh->indices.data(), indexBufferSize);

	core::gpu::BufferCreateInfo vertexInfo{
		.size = vertexBufferSize,
		.usage = core::BufferUsage::VertexBuffer | core::BufferUsage::TransferDst,
		.memoryProperties = core::MemoryProperty::DeviceLocal
	};

	buffers.vertexBuffer = std::make_unique<core::gpu::Buffer>(
		m_device.GetHandle(),
		m_device.GetPhysicalDevice(),
		vertexInfo
	);

	core::gpu::BufferCreateInfo indexInfo{
		.size = indexBufferSize,
		.usage = core::BufferUsage::IndexBuffer | core::BufferUsage::TransferDst,
		.memoryProperties = core::MemoryProperty::DeviceLocal
	};

	buffers.indexBuffer = std::make_unique<core::gpu::Buffer>(
		m_device.GetHandle(),
		m_device.GetPhysicalDevice(),
		indexInfo
	);

	buffers.indexCount = static_cast<uint32_t>(mesh->indices.size());

	core::gpu::CommandBufferCreateInfo cmdInfo{};
	cmdInfo.commandPool = m_device.GetCommandPool();
	cmdInfo.level = core::CommandBufferLevel::Primary;
	cmdInfo.count = 1;
	cmdInfo.singleTime = true;

	auto transferCmd = std::make_unique<core::gpu::CommandBuffer>(
		m_device.GetHandle(),
		m_device.GetGraphicsQueue(),
		cmdInfo
	);

	transferCmd->Begin(0);
	transferCmd->CopyBuffer(
		stagingVertexBuffer->GetHandle(),
		buffers.vertexBuffer->GetHandle(),
		vertexBufferSize
	);
	transferCmd->CopyBuffer(
		stagingIndexBuffer->GetHandle(),
		buffers.indexBuffer->GetHandle(),
		indexBufferSize
	);
	transferCmd->End(0);

	transferCmd->SubmitAndWait();

	m_meshBuffers[mesh.get()] = std::move(buffers);
}

void Renderer::UpdateCamera(const glm::mat4& view, const glm::mat4& proj, const glm::vec3& position)
{
	m_viewMatrix = view;
	m_projMatrix = proj;
	m_cameraPosition = position;
}

void Renderer::UpdateUniformBuffer(uint32_t frameIndex, const std::shared_ptr<resources::object::Material>& material)
{
	core::gpu::UniformBufferObject ubo{};

	ubo.view = m_viewMatrix;
	ubo.proj = m_projMatrix;
	ubo.viewPos = m_cameraPosition;

	if (m_scene)
	{
		ubo.numLights = std::min(static_cast<int>(m_scene->lights.size()),
			core::gpu::MAX_LIGHTS);

		for (int i = 0; i < ubo.numLights; i++)
		{
			const auto& light = m_scene->lights[i];
			ubo.lights[i].position = light.position;
			ubo.lights[i].color = light.color;
			ubo.lights[i].intensity = light.intensity;
			ubo.lights[i].enabled = light.enabled ? 1 : 0;
			ubo.lights[i].type = 0;
			ubo.lights[i].lightRadius = light.radius;
		}
	}
	else
	{
		ubo.numLights = 0;
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

void Renderer::RecordCommandBuffer(uint32_t frameIndex, uint32_t imageIndex)
{
	auto& cmd = m_commandBuffers[frameIndex];

	cmd->Begin(0);

	uint32_t width = m_device.GetSwapchainWidth();
	uint32_t height = m_device.GetSwapchainHeight();

	void* colorImageHandle = m_device.GetColorImage();
	void* swapchainImageHandle = m_device.GetSwapchainImage(imageIndex);
	void* depthImageHandle = m_device.GetDepthImage();

	cmd->TransitionImageLayout(
		colorImageHandle,
		core::ImageLayout::Undefined,
		core::ImageLayout::ColorAttachment,
		false
	);

	cmd->TransitionImageLayout(
		swapchainImageHandle,
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
		width, height,
		m_device.GetColorImageView(),
		m_device.GetDepthImageView()
	);

	cmd->BindPipeline(m_device.GetPipeline());
	cmd->SetViewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
	cmd->SetScissor(0, 0, width, height);

	if (m_scene)
	{
		const resources::object::Material* lastMaterial = nullptr;

		auto staticMeshes = m_scene->GetStaticMeshes();
		for (auto* staticMesh : staticMeshes)
		{
			if (!staticMesh->visible) continue;

			auto& mesh = staticMesh->mesh;
			auto& material = staticMesh->material;

			if (!mesh || !material) continue;

			if (material.get() != lastMaterial)
			{
				UpdateUniformBuffer(frameIndex, material);
				cmd->BindDescriptorSets(
					m_device.GetPipelineLayout(),
					m_device.GetDescriptorSet(frameIndex),
					0
				);
				lastMaterial = material.get();
			}

			PushConstants pushConstants;
			pushConstants.model = staticMesh->GetTransformMatrix();

			cmd->PushConstants(
				m_device.GetPipelineLayout(),
				static_cast<uint32_t>(core::ShaderStageFlags::Vertex),
				0,
				sizeof(PushConstants),
				&pushConstants
			);

			auto it = m_meshBuffers.find(mesh.get());
			if (it != m_meshBuffers.end())
			{
				const auto& buffers = it->second;
				cmd->BindVertexBuffer(buffers.vertexBuffer->GetHandle());
				cmd->BindIndexBuffer(buffers.indexBuffer->GetHandle());
				cmd->DrawIndexed(buffers.indexCount);
			}
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
		swapchainImageHandle,
		width,
		height
	);

	cmd->TransitionImageLayout(
		swapchainImageHandle,
		core::ImageLayout::TransferDst,
		core::ImageLayout::Present,
		false
	);

	cmd->End(0);
}

void Renderer::DrawFrame()
{
	if (!m_running) return;

	UpdateCameraFromScene();
	m_device.BeginFrame(m_currentFrame);

	uint32_t imageIndex = m_device.AcquireNextImage(m_currentFrame);
	if (imageIndex == UINT32_MAX)
	{
		m_device.RecreateSwapchain();
		return;
	}

	RecordCommandBuffer(m_currentFrame, imageIndex);

	void* waitSemaphore = m_device.GetImageAvailableSemaphore(m_currentFrame);
	void* signalSemaphore = m_device.GetRenderFinishedSemaphore(imageIndex);
	void* fence = m_device.GetInFlightFence(m_currentFrame);

	m_commandBuffers[m_currentFrame]->Submit(waitSemaphore, signalSemaphore, fence);

	m_device.Present(imageIndex);

	m_currentFrame = (m_currentFrame + 1) % core::gpu::Device::FRAMES_IN_FLIGHT;
	m_frameCounter++;
}

void Renderer::Cleanup()
{
	m_device.WaitIdle();

	if (!m_running) return;
	m_running = false;
	m_meshBuffers.clear();
	m_commandBuffers.clear();
	m_device.Cleanup();
}

void Renderer::EnableRayTracing()
{
	if (m_rayTracingEnabled) return;

	std::cout << "Enabling ray tracing..." << std::endl;

	if (!m_scene)
	{
		std::cerr << "Cannot enable ray tracing: no scene set!" << std::endl;
		return;
	}

	auto staticMeshes = m_scene->GetStaticMeshes();
	for (auto* staticMesh : staticMeshes)
	{
		if (!staticMesh->mesh) continue;

		if (m_rtMeshData.find(staticMesh->mesh.get()) == m_rtMeshData.end())
		{
			CreateRTMeshBuffers(staticMesh->mesh);
			CreateBLAS(staticMesh->mesh.get());
		}
	}

	BuildTLAS();

	if (m_tlas)
	{
		void* tlasHandle = m_tlas->GetHandle();
		for (uint32_t i = 0; i < core::gpu::Device::FRAMES_IN_FLIGHT; i++)
		{
			m_device.UpdateDescriptorWithTLAS(i, tlasHandle);
		}
		std::cout << "TLAS bound to all descriptor sets!" << std::endl;
	}

	m_rayTracingEnabled = true;
	std::cout << "Ray tracing enabled successfully!" << std::endl;
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

	core::gpu::BufferCreateInfo rtVertexInfo{
		.size = vertexBufferSize,
		.usage = core::BufferUsage::AccelerationStructureBuildInput |
				 core::BufferUsage::ShaderDeviceAddress |
				 core::BufferUsage::StorageBuffer,
		.memoryProperties = core::MemoryProperty::HostVisible | core::MemoryProperty::HostCoherent
	};

	rtData.rtVertexBuffer = std::make_unique<core::gpu::Buffer>(
		m_device.GetHandle(),
		m_device.GetPhysicalDevice(),
		rtVertexInfo
	);
	rtData.rtVertexBuffer->CopyFrom(mesh->vertices.data(), vertexBufferSize);

	core::gpu::BufferCreateInfo rtIndexInfo{
		.size = indexBufferSize,
		.usage = core::BufferUsage::AccelerationStructureBuildInput |
				 core::BufferUsage::ShaderDeviceAddress |
				 core::BufferUsage::StorageBuffer,
		.memoryProperties = core::MemoryProperty::HostVisible | core::MemoryProperty::HostCoherent
	};

	rtData.rtIndexBuffer = std::make_unique<core::gpu::Buffer>(
		m_device.GetHandle(),
		m_device.GetPhysicalDevice(),
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

	core::gpu::AccelerationStructureGeometry geometry{};
	geometry.vertexBuffer = rtData.rtVertexBuffer.get();
	geometry.vertexCount = static_cast<uint32_t>(mesh->vertices.size());
	geometry.vertexStride = sizeof(resources::object::Vertex);
	geometry.indexBuffer = rtData.rtIndexBuffer.get();
	geometry.indexCount = static_cast<uint32_t>(mesh->indices.size());
	geometry.triangleCount = geometry.indexCount / 3;
	geometry.opaque = true;

	core::gpu::AccelerationStructureCreateInfo blasInfo{};
	blasInfo.type = core::gpu::AccelerationStructureType::BottomLevel;
	blasInfo.geometries.push_back(geometry);
	blasInfo.preferFastTrace = true;
	blasInfo.allowUpdate = false;

	rtData.blas = std::make_unique<core::gpu::AccelerationStructure>(
		m_device.GetHandle(),
		m_device.GetPhysicalDevice(),
		blasInfo
	);
}

void Renderer::BuildTLAS()
{
	if (!m_scene)
	{
		std::cerr << "Cannot build TLAS: no scene set!" << std::endl;
		return;
	}

	auto staticMeshes = m_scene->GetStaticMeshes();
	if (staticMeshes.empty())
	{
		std::cerr << "Cannot build TLAS: no mesh instances!" << std::endl;
		return;
	}

	std::vector<core::gpu::AccelerationStructureInstance> instances;
	instances.reserve(staticMeshes.size());

	uint32_t instanceIndex = 0;
	for (auto* staticMesh : staticMeshes)
	{
		if (!staticMesh->visible || !staticMesh->mesh) continue;

		auto it = m_rtMeshData.find(staticMesh->mesh.get());
		if (it == m_rtMeshData.end() || !it->second.blas)
		{
			std::cerr << "Warning: BLAS not found for mesh instance!" << std::endl;
			continue;
		}

		const glm::mat4 mat = staticMesh->GetTransformMatrix();
		float transform[3][4] = {
			{mat[0][0], mat[1][0], mat[2][0], mat[3][0]},
			{mat[0][1], mat[1][1], mat[2][1], mat[3][1]},
			{mat[0][2], mat[1][2], mat[2][2], mat[3][2]}
		};

		core::gpu::AccelerationStructureInstance instance{};
		std::memcpy(&instance.transform, &transform, sizeof(transform));
		instance.instanceCustomIndex = instanceIndex++;
		instance.mask = 0xFF;
		instance.instanceShaderBindingTableRecordOffset = 0;
		instance.blas = it->second.blas.get();

		instances.push_back(instance);
	}

	if (instances.empty())
	{
		std::cerr << "No valid instances for TLAS!" << std::endl;
		return;
	}

	core::gpu::AccelerationStructureCreateInfo tlasInfo{};
	tlasInfo.type = core::gpu::AccelerationStructureType::TopLevel;
	tlasInfo.instances = instances;
	tlasInfo.preferFastTrace = true;
	tlasInfo.allowUpdate = false;

	m_tlas = std::make_unique<core::gpu::AccelerationStructure>(
		m_device.GetHandle(),
		m_device.GetPhysicalDevice(),
		tlasInfo
	);

	RebuildAccelerationStructures();
}

void Renderer::RebuildAccelerationStructures()
{
	core::gpu::CommandBufferCreateInfo cmdInfo{};
	cmdInfo.commandPool = m_device.GetCommandPool();
	cmdInfo.count = 1;
	cmdInfo.singleTime = true;
	cmdInfo.level = core::CommandBufferLevel::Primary;

	core::gpu::CommandBuffer cmdBuffer(
		m_device.GetHandle(),
		m_device.GetGraphicsQueue(),
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
	cmdBuffer.SubmitAndWait();

	std::cout << "Acceleration structures built successfully!" << std::endl;
}

void Renderer::UpdateCameraFromScene()
{
	if (m_scene && m_scene->activeCamera)
	{
		UpdateCamera(
			m_scene->activeCamera->GetViewMatrix(),
			m_scene->activeCamera->GetProjectionMatrix(),
			m_scene->activeCamera->GetPosition()
		);
	}
}