#include <graphics/renderer.h>
#include <graphics/renderPass/gBufferPass.h>
#include <graphics/renderPass/lightingPass.h>

#include <core/gpu/buffer.h>
#include <core/gpu/descriptorSet.h>
#include <core/gpu/pipeline.h>
#include <core/enum.h>
#include <loaders/shaderLoader.h>
#include <loaders/textureLoader.h>

#include <memory>
#include <iostream>

using namespace core;
using namespace core::gpu;
using namespace graphics;

Renderer::Renderer(Window& window, Device& device)
	: m_window(window)
	, m_device(device)
	, m_currentFrame(0)
	, m_running(true)
	, m_frameCounter(0)
	, m_viewMatrix(glm::mat4(1.0f))
	, m_projMatrix(glm::mat4(1.0f))
	, m_cameraPosition(glm::vec3(0.0f))
{
	CreateCommandBuffers();
	CreateUniformBuffers();
	InitPasses();
	m_tlasPerFrame.resize(Device::s_FRAMES_IN_FLIGHT);
}

Renderer::~Renderer()
{
	Cleanup();
}

void Renderer::InitPasses()
{
	auto gBufferPass = std::make_unique<GBufferPass>(m_device, uniformBuffers);
	m_gBufferPass = gBufferPass.get();
	m_passes.push_back(std::move(gBufferPass));

	auto lightingPass = std::make_unique<LightingPass>(m_device, uniformBuffers);
	m_lightingPass = lightingPass.get();
	m_passes.push_back(std::move(lightingPass));

	m_lightingPass->SetGBufferInputs(
		m_gBufferPass->GetColorAttachments(),
		*m_gBufferPass->GetDepthAttachment()
	);
}

void Renderer::SetCamera(const glm::mat4& view, const glm::mat4& projection)
{
	m_viewMatrix = view;
	m_projMatrix = projection;
	m_cameraPosition = glm::vec3(glm::inverse(view)[3]);
}

void Renderer::PushMesh(resources::Mesh* mesh, const glm::mat4& transform)
{
	if (!mesh) return;
	if (!mesh->blas)
		std::cerr << "Warning: Mesh pushed without BLAS!\n";
	m_meshInstances.push_back({ mesh, transform });
}

void Renderer::PushLight(const resources::Light& light)
{
	m_lights.push_back(light);
}

void Renderer::BuildTLAS()
{
	if (m_meshInstances.empty()) return;

	std::vector<SAccelerationStructureInstance> instances;
	instances.reserve(m_meshInstances.size());

	uint32_t instanceIndex = 0;
	for (const auto& meshInstance : m_meshInstances)
	{
		if (!meshInstance.first->blas)
		{
			std::cerr << "Warning: BLAS not found for mesh instance!\n";
			continue;
		}

		const glm::mat4& mat = meshInstance.second;
		float transform[3][4] = {
			{mat[0][0], mat[1][0], mat[2][0], mat[3][0]},
			{mat[0][1], mat[1][1], mat[2][1], mat[3][1]},
			{mat[0][2], mat[1][2], mat[2][2], mat[3][2]}
		};

		SAccelerationStructureInstance instance{};
		std::memcpy(&instance.transform, &transform, sizeof(transform));
		instance.instanceCustomIndex = instanceIndex++;
		instance.mask = 0xFF;
		instance.instanceShaderBindingTableRecordOffset = 0;
		instance.blas = meshInstance.first->blas.get();

		if (meshInstance.first->blas->GetDeviceAddress() == 0)
		{
			std::cerr << "ERROR: BLAS has invalid device address!\n";
			continue;
		}

		instances.push_back(instance);
	}

	if (instances.empty()) return;

	SAccelerationStructureCreateInfo tlasInfo{};
	tlasInfo.type = EAccelerationStructureType::TopLevel;
	tlasInfo.instances = instances;
	tlasInfo.preferFastTrace = true;
	tlasInfo.allowUpdate = false;

	m_tlasPerFrame[m_currentFrame] = std::make_unique<AccelerationStructure>(&m_device, tlasInfo);
}

void Renderer::RebuildAccelerationStructures()
{
	SCommandBufferCreateInfo cmdInfo{};
	cmdInfo.device = &m_device;
	cmdInfo.count = 1;
	cmdInfo.singleTime = true;
	cmdInfo.level = ECommandBufferLevel::Primary;

	gpu::CommandBuffer cmdBuffer(&m_device, cmdInfo);
	cmdBuffer.Begin(0);

	if (m_tlasPerFrame[m_currentFrame])
		cmdBuffer.BuildAccelerationStructure(m_tlasPerFrame[m_currentFrame].get());

	cmdBuffer.End(0);
	cmdBuffer.SubmitImmediate(&m_device);
}

void Renderer::CreateUniformBuffers()
{
	uniformBuffers.clear();
	uniformBuffers.reserve(Device::s_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < Device::s_FRAMES_IN_FLIGHT; i++)
	{
		SBufferCreateInfo bufferInfo{
			.size = sizeof(UniformBufferObject),
			.usage = EBufferUsage::UniformBuffer,
			.memoryProperties = EMemoryProperty::HostVisible | EMemoryProperty::HostCoherent
		};
		uniformBuffers.push_back(std::make_unique<Buffer>(&m_device, bufferInfo));
	}
}

void Renderer::UpdateUniformBuffer(uint32_t frameIndex)
{
	UniformBufferObject ubo{};

	ubo.view = m_viewMatrix;
	ubo.proj = m_projMatrix;
	ubo.viewPos = glm::vec4(m_cameraPosition, 1.0f);
	ubo.viewProjInverse = glm::inverse(m_projMatrix * m_viewMatrix);

	ubo.numLights = std::min(static_cast<int>(m_lights.size()), MAX_LIGHTS);
	for (int i = 0; i < ubo.numLights; i++)
	{
		const auto& light = m_lights[i];
		ubo.lights[i].position = glm::vec4(light.position, 1.0f);
		ubo.lights[i].color = glm::vec4(light.color, 1.0f);
		ubo.lights[i].intensity = light.intensity;
		ubo.lights[i].enabled = light.enabled ? 1 : 0;
		ubo.lights[i].type = 0;
		ubo.lights[i].lightRadius = light.radius;
	}

	ubo.frameCount = static_cast<uint32_t>(m_frameCounter);

	if (uniformBuffers[frameIndex])
	{
		uniformBuffers[frameIndex]->CopyFrom(&ubo, sizeof(UniformBufferObject));
	}
}

void Renderer::OnResize()
{
	m_device.WaitIdle();
	m_passes.clear();
	m_gBufferPass = nullptr;
	m_lightingPass = nullptr;
	InitPasses();
}

void Renderer::CreateCommandBuffers()
{
	m_commandBuffers.clear();
	m_commandBuffers.reserve(Device::s_FRAMES_IN_FLIGHT);

	for (uint32_t i = 0; i < Device::s_FRAMES_IN_FLIGHT; i++)
	{
		SCommandBufferCreateInfo cmdInfo{};
		cmdInfo.device = &m_device;
		cmdInfo.level = ECommandBufferLevel::Primary;
		cmdInfo.count = 1;
		m_commandBuffers.push_back(std::make_unique<CommandBuffer>(&m_device, cmdInfo));
	}
}

void Renderer::Render(core::gpu::Image* outputImage, ImageLayout outputLayout)
{
	if (!m_running) return;

	m_device.BeginFrame(m_currentFrame);

	BuildTLAS();
	RebuildAccelerationStructures();
	UpdateUniformBuffer(m_currentFrame);

	auto& cmd = m_commandBuffers[m_currentFrame];
	cmd->Begin(0);


	//G-Buffer Pass
	m_gBufferPass->SetMeshInstances(&m_meshInstances);
	m_gBufferPass->UpdateDescriptorSets(m_currentFrame);
	m_gBufferPass->BindDescriptorSets(*cmd, m_currentFrame);

	std::vector<ColorAttachmentDesc> colorDescs;
	for (const auto& ca : m_gBufferPass->GetColorAttachments())
	{
		ColorAttachmentDesc desc{};
		desc.image = ca.image.get();
		desc.clear = true;
		colorDescs.push_back(desc);
	}

	DepthAttachmentDesc depthDesc{};
	if (const auto* depth = m_gBufferPass->GetDepthAttachment())
	{
		depthDesc.image = depth->image.get();
		depthDesc.clear = true;
		depthDesc.clearDepth = 1.0f;
	}

	m_gBufferPass->Draw(*cmd, colorDescs, depthDesc);

	//Lighting Pass
	if (m_tlasPerFrame[m_currentFrame])
	{
		m_lightingPass->SetTLAS(m_tlasPerFrame[m_currentFrame].get());
	}

	m_lightingPass->UpdateDescriptorSets(m_currentFrame);
	m_lightingPass->BindDescriptorSets(*cmd, m_currentFrame);

	std::vector<ColorAttachmentDesc> lightColorDescs;
	for (const auto& colorAttachments : m_lightingPass->GetColorAttachments())
	{
		ColorAttachmentDesc desc{};
		desc.image = colorAttachments.image.get();
		desc.clear = true;
		lightColorDescs.push_back(desc);
	}

	DepthAttachmentDesc lightDepthDesc{};
	if (const auto* depth = m_lightingPass->GetDepthAttachment())
	{
		lightDepthDesc.image = depth->image.get();
		lightDepthDesc.clear = true;
		lightDepthDesc.clearDepth = 1.0f;
	}

	m_lightingPass->Draw(*cmd, lightColorDescs, lightDepthDesc);

	if (outputImage)
	{
		cmd->TransitionImageLayout(
			outputImage,
			ImageLayout::Undefined,
			ImageLayout::TransferDst,
			false
		);

		if (m_lightingPass && !m_lightingPass->GetColorAttachments().empty())
		{
			cmd->BlitImage(
				m_lightingPass->GetColorAttachments()[0].image.get(),
				outputImage,
				&m_device
			);
		}

		cmd->TransitionImageLayout(
			outputImage,
			ImageLayout::TransferDst,
			outputLayout,
			false
		);
	}

	m_meshInstances.clear();
	m_lights.clear();
}
void graphics::Renderer::DrawScene(core::gpu::CommandBuffer* _cmd)
{
	if (!m_gBufferPass) return;

	for (const auto& meshInstance : m_meshInstances)
	{
		if (!meshInstance.first->vertexBuffer || !meshInstance.first->indexBuffer)
		{
			continue;
		}

		PushConstants pushConstants;
		pushConstants.model = meshInstance.second;

		_cmd->PushConstants(
			m_gBufferPass->GetPipeline(),
			static_cast<uint32_t>(core::ShaderStageFlags::Vertex),
			0,
			sizeof(PushConstants),
			&pushConstants
		);

		_cmd->BindVertexBuffer(meshInstance.first->vertexBuffer.get());
		_cmd->BindIndexBuffer(meshInstance.first->indexBuffer.get());
		_cmd->DrawIndexed(meshInstance.first->indexCount);
	}
}

void Renderer::Cleanup()
{
	m_device.WaitIdle();

	if (!m_running) return;
	m_running = false;

	m_passes.clear();
	m_gBufferPass = nullptr;
	m_lightingPass = nullptr;

	m_tlasPerFrame.clear();
	m_commandBuffers.clear();
	m_device.Cleanup();
}

CommandBuffer* graphics::Renderer::GetCurrentCommandBuffer()
{
	return m_commandBuffers[m_currentFrame].get();
}

void graphics::Renderer::Advance()
{
	m_commandBuffers[m_currentFrame]->End(0);
	m_commandBuffers[m_currentFrame]->Submit(&m_device, m_currentFrame);
	m_currentFrame = (m_currentFrame + 1) % Device::s_FRAMES_IN_FLIGHT;
	m_frameCounter++;
}
