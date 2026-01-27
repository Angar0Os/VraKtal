#include <graphics/renderer.h>
#include <core/gpu/imguiContext.h>

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
	m_tlasPerFrame.resize(core::gpu::Device::s_FRAMES_IN_FLIGHT);
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

void Renderer::SetCamera(const glm::mat4& view, const glm::mat4& projection)
{
	m_viewMatrix = view;
	m_projMatrix = projection;

	glm::mat4 invView = glm::inverse(view);
	m_cameraPosition = glm::vec3(invView[3]);
}

void Renderer::PushMesh(resources::Mesh* mesh, const glm::mat4& transform)
{
	if (!mesh) return;

	if (!mesh->blas)
	{
		std::cerr << "Warning: Mesh pushed without BLAS!" << std::endl;
	}

	m_meshInstances.push_back(std::make_pair(mesh, transform));
}

void Renderer::PushLight(const resources::Light& light)
{
	m_lights.push_back(light);
}

void Renderer::BuildTLAS()
{
	if (m_meshInstances.empty())
	{
		return;
	}

	std::vector<core::gpu::SAccelerationStructureInstance> instances;
	instances.reserve(m_meshInstances.size());

	uint32_t instanceIndex = 0;

	for (const auto& meshInstance : m_meshInstances)
	{
		if (!meshInstance.first->blas)
		{
			std::cerr << "Warning: BLAS not found for mesh instance!" << std::endl;
			continue;
		}

		const glm::mat4& mat = meshInstance.second;

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
		instance.blas = meshInstance.first->blas.get();

		instances.push_back(instance);

		uint64_t blasAddr = meshInstance.first->blas->GetDeviceAddress();

		if (blasAddr == 0)
		{
			std::cerr << "ERROR: BLAS has invalid device address!" << std::endl;
			continue;
		}
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

	m_tlasPerFrame[m_currentFrame] = std::make_unique<core::gpu::AccelerationStructure>(
		&m_device,
		tlasInfo
	);
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

	if (m_tlasPerFrame[m_currentFrame])
	{
		cmdBuffer.BuildAccelerationStructure(m_tlasPerFrame[m_currentFrame].get());
	}

	cmdBuffer.End(0);
	cmdBuffer.SubmitAndWait(&m_device);
}

void Renderer::UpdateUniformBuffer(uint32_t frameIndex)
{
	core::gpu::UniformBufferObject ubo{};

	ubo.view = m_viewMatrix;
	ubo.proj = m_projMatrix;
	ubo.viewPos = m_cameraPosition;

	ubo.numLights = std::min(static_cast<int>(m_lights.size()),
		core::gpu::MAX_LIGHTS);

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

	ubo.frameCount = static_cast<uint32_t>(m_frameCounter);

	auto* uniformBuffer = m_device.GetUniformBuffer(frameIndex);
	if (uniformBuffer)
	{
		uniformBuffer->CopyFrom(&ubo, sizeof(core::gpu::UniformBufferObject));
	}
}

void Renderer::Render(const core::gpu::Image* image, uint32_t imageIndex)
{
	if (!m_running) return;

	m_device.BeginFrame(m_currentFrame);

	BuildTLAS();
	RebuildAccelerationStructures();

	if (m_tlasPerFrame[m_currentFrame])
	{
		m_device.UpdateDescriptorWithTLAS(m_currentFrame, m_tlasPerFrame[m_currentFrame].get());
	}

	UpdateUniformBuffer(m_currentFrame);

	auto& cmd = m_commandBuffers[m_currentFrame];

	cmd->Begin(0);

	const auto* colorImageHandle = m_device.GetColorImage();
	const auto* swapchainImageHandle = m_device.GetSwapchainImage(imageIndex);
	const auto* depthImageHandle = m_device.GetDepthImage();

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
		&m_device,
		colorImageHandle,
		depthImageHandle
	);

	cmd->BindPipeline(m_device.GetGraphicsPipeline());
	cmd->SetViewport(0.0f, 0.0f, &m_device);
	cmd->SetScissor(0, 0, &m_device);

	cmd->BindDescriptorSets(
		&m_device,
		m_currentFrame,
		0
	);

	for (const auto& meshInstance : m_meshInstances)
	{
		if (!meshInstance.first->vertexBuffer || !meshInstance.first->indexBuffer)
		{
			continue;
		}

		PushConstants pushConstants;
		pushConstants.model = meshInstance.second;

		cmd->PushConstants(
			m_device.GetGraphicsPipeline(),
			static_cast<uint32_t>(core::ShaderStageFlags::Vertex),
			0,
			sizeof(PushConstants),
			&pushConstants
		);

		cmd->BindVertexBuffer(meshInstance.first->vertexBuffer.get());
		cmd->BindIndexBuffer(meshInstance.first->indexBuffer.get());
		cmd->DrawIndexed(meshInstance.first->indexCount);
	}

	m_device.GetImGuiContext()->PrepareDrawData();
	m_device.GetImGuiContext()->DrawEditors(static_cast<void*>(cmd.get()));

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
		&m_device
	);

	cmd->TransitionImageLayout(
		swapchainImageHandle,
		core::ImageLayout::TransferDst,
		core::ImageLayout::Present,
		false
	);

	cmd->End(0);

	void* waitSemaphore = m_device.GetImageAvailableSemaphore(m_currentFrame);
	void* signalSemaphore = m_device.GetRenderFinishedSemaphore(imageIndex);
	void* fence = m_device.GetInFlightFence(m_currentFrame);

	m_commandBuffers[m_currentFrame]->Submit(&m_device, waitSemaphore, signalSemaphore, fence);

	m_currentFrame = (m_currentFrame + 1) % core::gpu::Device::s_FRAMES_IN_FLIGHT;
	m_frameCounter++;

	m_meshInstances.clear();
	m_lights.clear();
}

void Renderer::Cleanup()
{
	m_device.WaitIdle();

	if (!m_running) return;
	m_running = false;

	m_tlasPerFrame.clear();
	m_commandBuffers.clear();
	m_device.Cleanup();
}