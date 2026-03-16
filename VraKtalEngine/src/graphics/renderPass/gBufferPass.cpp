#include <graphics/renderPass/gBufferPass.h>

#include <core/gpu/descriptorSet.h>
#include <core/enum.h>
#include <loaders/shaderLoader.h>

#include <iostream>

using namespace core;
using namespace core::gpu;
using namespace graphics::resources;

graphics::GBufferPass::GBufferPass(Device& device,
	const std::vector<std::unique_ptr<Buffer>>& uniformBuffers)
	: Pass("GBuffer")
	, m_device(device)
	, m_uniformBuffers(uniformBuffers)
{
	Init(device);
}

void graphics::GBufferPass::Init(Device& device)
{
	if (m_debugEnabled)
		std::cout << "[Pass:" << m_name << "] Init\n";

	CreateAttachments();
	CreateDescriptorSetLayout();
	CreatePipeline();
	CreateDescriptorSets();
}

void graphics::GBufferPass::CreateAttachments()
{
	auto [width, height] = m_device.GetSwapchainExtent();

	const TextureFormat colorFormats[] = {
		TextureFormat::RGBA8_UNorm,
		TextureFormat::RGBA16_Float,
	};

	m_colorAttachments.clear();
	m_colorAttachments.resize(2);

	for (int i = 0; i < 2; ++i)
	{
		SImageCreateInfo info{
			.width = width,
			.height = height,
			.mipLevels = 1,
			.format = colorFormats[i],
			.tiling = ImageTiling::Optimal,
			.usage = ImageUsage::ColorAttachment | ImageUsage::Sampled,
			.memoryProperties = EMemoryProperty::DeviceLocal,
			.samples = SampleCount::e1
		};

		m_colorAttachments[i].image = std::make_unique<Image>(&m_device, info);
		m_colorAttachments[i].texture = std::make_unique<Texture>(m_device,
			*m_colorAttachments[i].image);
	}

	{
		SImageCreateInfo depthInfo{
			.width = width,
			.height = height,
			.mipLevels = 1,
			.format = TextureFormat::Depth32F,
			.tiling = ImageTiling::Optimal,
			.usage = ImageUsage::DepthStencilAttachment | ImageUsage::Sampled,
			.memoryProperties = EMemoryProperty::DeviceLocal,
			.samples = SampleCount::e1
		};

		m_depthAttachment.image = std::make_unique<Image>(&m_device, depthInfo);
		m_depthAttachment.texture = std::make_unique<Texture>(m_device,
			*m_depthAttachment.image);
	}
}

void graphics::GBufferPass::CreateDescriptorSetLayout()
{
	SDescriptorSetLayoutBinding uboBinding{
		.binding = 0,
		.descriptorType = EDescriptorType::UniformBuffer,
		.stageFlags = core::ShaderStage::Vertex | core::ShaderStage::Fragment
	};

	SDescriptorSetLayoutCreateInfo layoutInfo{
		.bindings = { uboBinding }
	};

	m_dsLayouts.clear();
	m_dsLayouts.push_back(
		std::make_unique<DescriptorSetLayout>(&m_device, layoutInfo));
}

void graphics::GBufferPass::CreatePipeline()
{
	auto shaderCode = loaders::ReadFile("../bin/assets/shaders/gbuffer.spv");

	SVertexInputBinding vertexBinding{
		.binding = 0,
		.stride = sizeof(resources::Vertex),
		.inputRate = VertexInputRate::Vertex
	};

	std::vector<SVertexInputAttribute> vertexAttributes = {
		{0, 0, TextureFormat::RGB32_Float, offsetof(resources::Vertex, position)},
		{1, 0, TextureFormat::RGB32_Float, offsetof(resources::Vertex, normal)},
		{2, 0, TextureFormat::RG32_Float,  offsetof(resources::Vertex, uv)}
	};

	std::vector<core::gpu::ShaderStage> shaderStages = {
		{ShaderStageFlags::Vertex,   shaderCode, "vertMain"},
		{ShaderStageFlags::Fragment, shaderCode, "fragMain"}
	};

	std::vector<PushConstantRange> pushConstants = {
		{
			.stageFlags = static_cast<uint32_t>(ShaderStageFlags::Vertex),
			.offset = 0,
			.size = sizeof(glm::mat4)
		}
	};

	std::vector<TextureFormat> colorFormats = {
		TextureFormat::RGBA8_UNorm,
		TextureFormat::RGBA16_Float,
	};

	PipelineCreateInfo pipelineInfo{};
	pipelineInfo.shaderStages = shaderStages;
	pipelineInfo.vertexBindings = { vertexBinding };
	pipelineInfo.vertexAttributes = vertexAttributes;
	pipelineInfo.topology = PrimitiveTopology::TriangleList;
	pipelineInfo.polygonMode = PolygonMode::Fill;
	pipelineInfo.cullMode = CullMode::Back;
	pipelineInfo.frontFace = FrontFace::Clockwise;
	pipelineInfo.depthTestEnable = true;
	pipelineInfo.depthWriteEnable = true;
	pipelineInfo.depthCompareOp = CompareOp::Less;
	pipelineInfo.blendEnable = false;
	pipelineInfo.samples = SampleCount::e1;
	pipelineInfo.colorAttachmentFormats = colorFormats;
	pipelineInfo.depthAttachmentFormat = TextureFormat::Depth32F;
	pipelineInfo.descriptorSetLayouts = { m_dsLayouts[0].get() };
	pipelineInfo.pushConstantRanges = pushConstants;
	pipelineInfo.dynamicStates = { DynamicState::Viewport, DynamicState::Scissor };

	m_pipeline = std::make_unique<Pipeline>(&m_device, pipelineInfo);
}

void graphics::GBufferPass::CreateDescriptorSets()
{
	m_descriptorSets.clear();
	m_descriptorSets.reserve(Device::s_FRAMES_IN_FLIGHT);

	for (uint32_t i = 0; i < Device::s_FRAMES_IN_FLIGHT; ++i)
	{
		auto ds = std::make_unique<DescriptorSet>(&m_device, m_dsLayouts[0].get());
		ds->Bind(0, *m_uniformBuffers[i]);
		ds->Update(m_device);
		m_descriptorSets.push_back(std::move(ds));
	}
}

void graphics::GBufferPass::UpdateDescriptorSets(uint32_t frameIndex)
{
	(void)frameIndex;
}

void graphics::GBufferPass::BindDescriptorSets(CommandBuffer& cmd, uint32_t frameIndex)
{
	cmd.BindDescriptorSets(
		m_pipeline.get(),
		m_descriptorSets[frameIndex].get(),
		frameIndex,
		0
	);
}

void graphics::GBufferPass::Draw(CommandBuffer& cmd,
	const std::vector<ColorAttachmentDesc>& colorAttachments,
	const DepthAttachmentDesc& depthAttachment)
{
	if (m_debugEnabled)
		std::cout << "[Pass:" << m_name << "] Draw\n";

	auto [width, height] = m_device.GetSwapchainExtent();

	for (auto& ca : m_colorAttachments)
	{
		cmd.TransitionImageLayout(
			ca.image.get(),
			ImageLayout::Undefined,
			ImageLayout::ColorAttachment,
			false
		);
	}

	cmd.TransitionImageLayout(
		m_depthAttachment.image.get(),
		ImageLayout::Undefined,
		ImageLayout::DepthStencilAttachment,
		true
	);

	std::vector<CommandBuffer::RenderingAttachmentInfo> colorInfos;
	colorInfos.reserve(m_colorAttachments.size());
	for (const auto& ca : m_colorAttachments)
	{
		CommandBuffer::RenderingAttachmentInfo info{};
		info.image = ca.image.get();
		info.clear = true;
		info.clearR = 0.0f;
		info.clearG = 0.0f;
		info.clearB = 0.0f;
		info.clearA = 1.0f;
		colorInfos.push_back(info);
	}

	CommandBuffer::DepthAttachmentInfo depthInfo{};
	depthInfo.image = m_depthAttachment.image.get();
	depthInfo.clear = true;
	depthInfo.clearDepth = 1.0f;

	cmd.BeginRendering(&m_device, colorInfos, depthInfo);

	cmd.BindPipeline(m_pipeline.get());
	cmd.SetViewport(0.0f, 0.0f, &m_device);
	cmd.SetScissor(0, 0, &m_device);

	if (m_meshInstances)
	{
		for (const auto& [mesh, transform] : *m_meshInstances)
		{
			if (!mesh->vertexBuffer || !mesh->indexBuffer)
				continue;

			glm::mat4 model = transform;
			cmd.PushConstants(
				m_pipeline.get(),
				static_cast<uint32_t>(core::ShaderStageFlags::Vertex),
				0,
				sizeof(glm::mat4),
				&model
			);

			cmd.BindVertexBuffer(mesh->vertexBuffer.get());
			cmd.BindIndexBuffer(mesh->indexBuffer.get());
			cmd.DrawIndexed(mesh->indexCount);
		}
	}

	cmd.EndRendering();

	for (auto& ca : m_colorAttachments)
	{
		cmd.TransitionImageLayout(
			ca.image.get(),
			ImageLayout::ColorAttachment,
			ImageLayout::ShaderReadOnly,
			false
		);
	}

	cmd.TransitionImageLayout(
		m_depthAttachment.image.get(),
		ImageLayout::DepthStencilAttachment,
		ImageLayout::ShaderReadOnly,
		true
	);
}

void graphics::GBufferPass::SetMeshInstances(
	const std::vector<std::pair<resources::Mesh*, glm::mat4>>* instances)
{
	m_meshInstances = instances;
}

const std::vector<graphics::PassAttachment>& graphics::GBufferPass::GetColorAttachments() const
{
	return m_colorAttachments;
}

const graphics::PassAttachment* graphics::GBufferPass::GetDepthAttachment() const
{
	return &m_depthAttachment;
}
