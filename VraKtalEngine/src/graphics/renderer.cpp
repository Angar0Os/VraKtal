#include <graphics/renderer.h>

#include <core/gpu/buffer.h>
#include <core/gpu/descriptorSet.h>
#include <core/gpu/imguiContext.h>
#include <core/gpu/buffer.h>
#include <core/gpu/pipeline.h>

#include <core/enum.h>

#include <loaders/shaderLoader.h>
#include <loaders/textureLoader.h>

#include <memory>
#include <iostream>

using namespace core;
using namespace core::gpu;
using namespace graphics;

//#define VRAKTAL_EDITOR

Renderer::Renderer(Window& window, Device& device)
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
	CreateUniformBuffers();

	CreateColorImage();
	CreateDepthImage();

	CreateDescriptorSetLayout();
	CreateGraphicsPipeline();
	CreateGraphicsDescriptorSet();

	m_tlasPerFrame.resize(Device::s_FRAMES_IN_FLIGHT);
}

Renderer::~Renderer()
{
	Cleanup();
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

		auto cmdBuffer = std::make_unique<CommandBuffer>(
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

	std::vector<SAccelerationStructureInstance> instances;
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

		SAccelerationStructureInstance instance{};
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

	SAccelerationStructureCreateInfo tlasInfo{};
	tlasInfo.type = EAccelerationStructureType::TopLevel;
	tlasInfo.instances = instances;
	tlasInfo.preferFastTrace = true;
	tlasInfo.allowUpdate = false;

	m_tlasPerFrame[m_currentFrame] = std::make_unique<AccelerationStructure>(
		&m_device,
		tlasInfo
	);
}

void Renderer::RebuildAccelerationStructures()
{
	SCommandBufferCreateInfo cmdInfo{};
	cmdInfo.device = &m_device;
	cmdInfo.count = 1;
	cmdInfo.singleTime = true;
	cmdInfo.level = ECommandBufferLevel::Primary;

	gpu::CommandBuffer cmdBuffer(
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

void Renderer::CreateDescriptorSetLayout()
{
	auto bindingUBO = SDescriptorSetLayoutBinding{};
	bindingUBO.binding = 0;
	bindingUBO.descriptorType = EDescriptorType::UniformBuffer;
	bindingUBO.stageFlags = core::ShaderStage::Vertex | core::ShaderStage::Fragment;
	
	auto bindingColorTexture = SDescriptorSetLayoutBinding{};
	bindingColorTexture.binding = 1;
	bindingColorTexture.descriptorType = EDescriptorType::CombinedImageSampler;
	bindingColorTexture.stageFlags = core::ShaderStage::Fragment;

	auto bindingDepthTexture = SDescriptorSetLayoutBinding{};
	bindingDepthTexture.binding = 2;
	bindingDepthTexture.descriptorType = EDescriptorType::CombinedImageSampler;
	bindingDepthTexture.stageFlags = core::ShaderStage::Fragment;

	auto layoutInfo = SDescriptorSetLayoutCreateInfo{};
	layoutInfo.bindings = { bindingUBO, bindingColorTexture, bindingDepthTexture };

	descriptorSetLayout = std::make_unique<DescriptorSetLayout>(&m_device, layoutInfo);
}

void Renderer::CreateGraphicsDescriptorSet()
{
	for (size_t i = 0; i < Device::s_FRAMES_IN_FLIGHT; i++)
	{
		auto descriptor = std::make_unique<DescriptorSet>(&m_device, descriptorSetLayout.get());

		descriptor->Bind(0, *uniformBuffers[i]);
		descriptor->Bind(1, *colorTexture);
		descriptor->Bind(2, *depthTexture);

		descriptor->Update(m_device);

		graphicsDescriptorSets.push_back(std::move(descriptor));
	}
}


void graphics::Renderer::CreateUniformBuffers()
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

void Renderer::CreateGraphicsPipeline()
{
	auto shaderCode = loaders::ReadFile("../bin/assets/shaders/slang.spv");

	SVertexInputBinding vertexBinding
	{
		.binding = 0,
		.stride = sizeof(graphics::resources::Vertex),
		.inputRate = VertexInputRate::Vertex
	};

	std::vector<SVertexInputAttribute> vertexAttributes = {
		{0, 0, TextureFormat::RGB32_Float, offsetof(graphics::resources::Vertex, position)},
		{1, 0, TextureFormat::RGB32_Float, offsetof(graphics::resources::Vertex, normal)},
		{2, 0, TextureFormat::RG32_Float, offsetof(graphics::resources::Vertex, uv)}
	};

	std::vector<core::gpu::ShaderStage> shaderStages = {
		{ShaderStageFlags::Vertex, shaderCode, "vertMain"},
		{ShaderStageFlags::Fragment, shaderCode, "fragMain"}
	};

	std::vector<PushConstantRange> pushConstants = {
		{
			.stageFlags = static_cast<uint32_t>(ShaderStageFlags::Vertex),
			.offset = 0,
			.size = sizeof(glm::mat4)
		}
	};

	PipelineCreateInfo pipelineInfo{};
	pipelineInfo.shaderStages = shaderStages;
	pipelineInfo.vertexBindings = { vertexBinding };
	pipelineInfo.vertexAttributes = vertexAttributes;
	pipelineInfo.topology = PrimitiveTopology::TriangleList;
	pipelineInfo.polygonMode = PolygonMode::Fill;
	pipelineInfo.cullMode = CullMode::None;
	pipelineInfo.frontFace = FrontFace::Clockwise;
	pipelineInfo.depthTestEnable = true;
	pipelineInfo.depthWriteEnable = true;
	pipelineInfo.depthCompareOp = CompareOp::Less;
	pipelineInfo.blendEnable = false;
	pipelineInfo.samples = SampleCount::e4;
	pipelineInfo.colorAttachmentFormats = { TextureFormat::RGBA8_SRGB };
	pipelineInfo.depthAttachmentFormat = TextureFormat::Depth32F;
	pipelineInfo.descriptorSetLayouts = { descriptorSetLayout.get() };
	pipelineInfo.pushConstantRanges = pushConstants;
	pipelineInfo.dynamicStates = { DynamicState::Viewport, DynamicState::Scissor };

	graphicsPipeline = std::make_unique<Pipeline>(&m_device, pipelineInfo);
}

void Renderer::UpdateUniformBuffer(uint32_t frameIndex)
{
	UniformBufferObject ubo{};

	ubo.view = m_viewMatrix;
	ubo.proj = m_projMatrix;
	ubo.viewPos = m_cameraPosition;

	ubo.numLights = std::min(static_cast<int>(m_lights.size()),
		MAX_LIGHTS);

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

	/*ubo.albedo = glm::vec3(1.0f);
	ubo.metallic = 0.0f;
	ubo.roughness = 0.5f;
	ubo.ao = 1.0f;
	ubo.emissive = glm::vec3(0.0f);

	ubo.useAlbedoMap = 0;
	ubo.useNormalMap = 0;
	ubo.useMetallicMap = 0;
	ubo.useRoughnessMap = 0;
	ubo.useAOMap = 0;
	ubo.useEmissiveMap = 0;*/

	ubo.frameCount = static_cast<uint32_t>(m_frameCounter);

	const auto& uniformBuffer = uniformBuffers[frameIndex];
	if (uniformBuffer)
	{
		uniformBuffer->CopyFrom(&ubo, sizeof(UniformBufferObject));
	}
}

void Renderer::Render(const Image* image, uint32_t imageIndex)
{
	// Le render crash car les images sont nulles, il faut recréer les images dans le renderer et les enlever du device.
	if (!m_running) return;

	m_device.BeginFrame(m_currentFrame);

	BuildTLAS();
	RebuildAccelerationStructures();

	//if (m_tlasPerFrame[m_currentFrame])
	//{
	//	m_device.UpdateDescriptorWithTLAS(m_currentFrame, m_tlasPerFrame[m_currentFrame].get());
	//}

	UpdateUniformBuffer(m_currentFrame);

	auto& cmd = m_commandBuffers[m_currentFrame];

	cmd->Begin(0);

	const auto* swapchainImageHandle = m_device.GetSwapchainImage(imageIndex);

	cmd->TransitionImageLayout(
		colorImage.get(),
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
		depthImage.get(),
		core::ImageLayout::Undefined,
		core::ImageLayout::DepthStencilAttachment,
		true
	);

	cmd->BeginRendering(
		&m_device,
		colorImage.get(),
		depthImage.get()
	);

	cmd->BindPipeline(graphicsPipeline.get());
	cmd->SetViewport(0.0f, 0.0f, &m_device);
	cmd->SetScissor(0, 0, &m_device);

	cmd->BindDescriptorSets(
		graphicsPipeline.get(),
		graphicsDescriptorSets[m_currentFrame].get(),
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
			graphicsPipeline.get(),
			static_cast<uint32_t>(core::ShaderStageFlags::Vertex),
			0,
			sizeof(PushConstants),
			&pushConstants
		);

		cmd->BindVertexBuffer(meshInstance.first->vertexBuffer.get());
		cmd->BindIndexBuffer(meshInstance.first->indexBuffer.get());
		cmd->DrawIndexed(meshInstance.first->indexCount);
	}

#ifdef VRAKTAL_EDITOR
	m_device.GetImGuiContext()->PrepareDrawData();
	m_device.GetImGuiContext()->DrawEditors(static_cast<void*>(cmd.get()));
#endif

	cmd->EndRendering();

	cmd->TransitionImageLayout(
		colorImage.get(),
		core::ImageLayout::ColorAttachment,
		core::ImageLayout::TransferSrc,
		false
	);

	cmd->ResolveImage(
		colorImage.get(),
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

	m_currentFrame = (m_currentFrame + 1) % Device::s_FRAMES_IN_FLIGHT;
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

void Renderer::CreateColorImage()
{
	SImageCreateInfo colorInfo{
		.width = m_device.GetSwapchainExtent().first,
		.height = m_device.GetSwapchainExtent().second,
		.mipLevels = 1,
		.format = TextureFormat::RGBA8_SRGB,
		.tiling = ImageTiling::Optimal,
		.usage = ImageUsage::ColorAttachment | ImageUsage::TransferSrc,
		.memoryProperties = EMemoryProperty::DeviceLocal,
		.samples = SampleCount::e4
	};

	colorImage = std::make_unique<Image>(&m_device, colorInfo);
	colorTexture = std::make_unique<Texture>(m_device, *colorImage);
}

void Renderer::CreateDepthImage()
{
	SImageCreateInfo depthInfo{
		.width = m_device.GetSwapchainExtent().first,
		.height = m_device.GetSwapchainExtent().second,
		.mipLevels = 1,
		.format = TextureFormat::Depth32F,
		.tiling = ImageTiling::Optimal,
		.usage = ImageUsage::DepthStencilAttachment,
		.memoryProperties = EMemoryProperty::DeviceLocal,
		.samples = SampleCount::e4
	};

	depthImage = std::make_unique<Image>(&m_device, depthInfo);
	depthTexture = std::make_unique<Texture>(m_device, *depthImage);
}
