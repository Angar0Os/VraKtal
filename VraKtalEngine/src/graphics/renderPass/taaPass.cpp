#include <graphics/renderPass/taaPass.h>

#include <core/gpu/descriptorSet.h>
#include <core/enum.h>

#include <loaders/shaderLoader.h>

using namespace core;
using namespace core::gpu;

graphics::TAAPass::TAAPass(Device& device,
	const std::vector<std::unique_ptr<Buffer>>& uniformBuffers)
	: Pass("TAA")
	, m_device(device)
	, m_uniformBuffers(uniformBuffers)
{
	Init(device);
}

void graphics::TAAPass::Init(Device& device)
{
	CreateAttachments();
	CreateDescriptorSetLayout();
	CreatePipeline();
	CreateDescriptorSets();
}

void graphics::TAAPass::CreateAttachments()
{
	auto [width, height] = m_device.GetSwapchainExtent();

	SImageCreateInfo info{
		.width = width,
		.height = height,
		.mipLevels = 1,
		.format = TextureFormat::RGBA16_Float, 
		.tiling = ImageTiling::Optimal,
		.usage = ImageUsage::ColorAttachment | ImageUsage::Sampled | ImageUsage::TransferDst,
		.memoryProperties = EMemoryProperty::DeviceLocal,
		.samples = SampleCount::e1
	};

	for (int i = 0; i < 2; ++i)
	{
		m_historyAttachments[i].image = std::make_unique<Image>(&m_device, info);
		m_historyAttachments[i].texture = std::make_unique<Texture>(m_device, *m_historyAttachments[i].image);
	}

	SImageCreateInfo outInfo = info;
	outInfo.usage = ImageUsage::ColorAttachment | ImageUsage::Sampled | ImageUsage::TransferSrc;

	m_colorAttachments.resize(1);
	m_colorAttachments[0].image = std::make_unique<Image>(&m_device, outInfo);
	m_colorAttachments[0].texture = std::make_unique<Texture>(m_device, *m_colorAttachments[0].image);
}

void graphics::TAAPass::CreateDescriptorSetLayout()
{
	SDescriptorSetLayoutBinding uboBinding{
		.binding = 0,
		.descriptorType = EDescriptorType::UniformBuffer,
		.stageFlags = core::ShaderStage::Fragment
	};
	SDescriptorSetLayoutBinding currentBinding{
		.binding = 1,
		.descriptorType = EDescriptorType::CombinedImageSampler,
		.stageFlags = core::ShaderStage::Fragment
	};
	SDescriptorSetLayoutBinding historyBinding{
		.binding = 2,
		.descriptorType = EDescriptorType::CombinedImageSampler,
		.stageFlags = core::ShaderStage::Fragment
	};
	SDescriptorSetLayoutBinding velocityBinding{
		.binding = 3,
		.descriptorType = EDescriptorType::CombinedImageSampler,
		.stageFlags = core::ShaderStage::Fragment
	};

	m_dsLayouts.clear();
	m_dsLayouts.push_back(std::make_unique<DescriptorSetLayout>(
		&m_device,
		SDescriptorSetLayoutCreateInfo{ .bindings = { uboBinding, currentBinding, historyBinding, velocityBinding } }
	));
}

void graphics::TAAPass::CreatePipeline()
{
	auto shaderCode = loaders::ReadFile("../bin/assets/shaders/taa.spv");

	PipelineCreateInfo pipelineInfo{};
	pipelineInfo.shaderStages = {
		{ShaderStageFlags::Vertex,   shaderCode, "vertMain"},
		{ShaderStageFlags::Fragment, shaderCode, "fragMain"}
	};
	pipelineInfo.vertexBindings = {};
	pipelineInfo.vertexAttributes = {};
	pipelineInfo.topology = PrimitiveTopology::TriangleList;
	pipelineInfo.polygonMode = PolygonMode::Fill;
	pipelineInfo.cullMode = CullMode::None;
	pipelineInfo.frontFace = FrontFace::Clockwise;
	pipelineInfo.depthTestEnable = false;
	pipelineInfo.depthWriteEnable = false;
	pipelineInfo.depthCompareOp = CompareOp::Always;
	pipelineInfo.blendEnable = false;
	pipelineInfo.samples = SampleCount::e1;
	pipelineInfo.colorAttachmentFormats = { TextureFormat::RGBA16_Float };
	pipelineInfo.depthAttachmentFormat = TextureFormat::Undefined;
	pipelineInfo.descriptorSetLayouts = { m_dsLayouts[0].get() };
	pipelineInfo.pushConstantRanges = {};
	pipelineInfo.dynamicStates = { DynamicState::Viewport, DynamicState::Scissor };

	m_pipeline = std::make_unique<Pipeline>(&m_device, pipelineInfo);
}

void graphics::TAAPass::CreateDescriptorSets()
{
	m_descriptorSets.clear();
	m_descriptorSets.reserve(Device::s_FRAMES_IN_FLIGHT);

	for (uint32_t i = 0; i < Device::s_FRAMES_IN_FLIGHT; ++i)
	{
		auto ds = std::make_unique<DescriptorSet>(&m_device, m_dsLayouts[0].get());
		ds->Bind(0, *m_uniformBuffers[i]);
		m_descriptorSets.push_back(std::move(ds));
	}
}

void graphics::TAAPass::SetInputs(const PassAttachment& currentColor,
	const PassAttachment& velocityTex)
{
	m_currentColor = &currentColor;
	m_velocityTex = &velocityTex;

	for (uint32_t i = 0; i < Device::s_FRAMES_IN_FLIGHT; ++i)
	{
		m_descriptorSets[i]->Bind(1, *m_currentColor->texture);
		m_descriptorSets[i]->Bind(3, *m_velocityTex->texture);
		m_descriptorSets[i]->Update(m_device);
	}
}

void graphics::TAAPass::UpdateDescriptorSets(uint32_t frameIndex)
{
	uint32_t historyReadIdx = (frameIndex + 1) % 2;

	m_descriptorSets[frameIndex]->Bind(2, *m_historyAttachments[historyReadIdx].texture);
	m_descriptorSets[frameIndex]->Update(m_device);
}

void graphics::TAAPass::BindDescriptorSets(CommandBuffer& cmd, uint32_t frameIndex)
{
	cmd.BindDescriptorSets(m_pipeline.get(), m_descriptorSets[frameIndex].get(), frameIndex, 0);
}

void graphics::TAAPass::Draw(CommandBuffer& cmd,
	const std::vector<ColorAttachmentDesc>&,
	const DepthAttachmentDesc&,
	uint32_t currentFrame)
{
	uint32_t historyWriteIdx = currentFrame % 2;

	cmd.TransitionImageLayout(
		m_historyAttachments[historyWriteIdx].image.get(),
		ImageLayout::Undefined,
		ImageLayout::ColorAttachment,
		false
	);
	cmd.TransitionImageLayout(
		m_colorAttachments[0].image.get(),
		ImageLayout::Undefined,
		ImageLayout::ColorAttachment,
		false
	);

	CommandBuffer::RenderingAttachmentInfo colorInfo{};
	colorInfo.image = m_colorAttachments[0].image.get();
	colorInfo.clear = false;

	cmd.BeginRendering(&m_device, { colorInfo }, {});
	cmd.BindPipeline(m_pipeline.get());
	cmd.SetViewport(0.0f, 0.0f, &m_device);
	cmd.SetScissor(0, 0, &m_device);

	UpdateDescriptorSets(currentFrame);
	BindDescriptorSets(cmd, currentFrame);

	cmd.DrawIndexed(3, 1, 0, 0, 0); 
	cmd.EndRendering();

	cmd.TransitionImageLayout(
		m_colorAttachments[0].image.get(),
		ImageLayout::ColorAttachment,
		ImageLayout::TransferSrc,
		false
	);
	cmd.TransitionImageLayout(
		m_historyAttachments[historyWriteIdx].image.get(),
		ImageLayout::ColorAttachment,
		ImageLayout::TransferDst,
		false
	);

	cmd.BlitImage(
		m_colorAttachments[0].image.get(),
		m_historyAttachments[historyWriteIdx].image.get(),
		&m_device
	);

	cmd.TransitionImageLayout(
		m_historyAttachments[historyWriteIdx].image.get(),
		ImageLayout::TransferDst,
		ImageLayout::ShaderReadOnly,
		false
	);
}

const std::vector<graphics::PassAttachment>& graphics::TAAPass::GetColorAttachments() const
{
	return m_colorAttachments;
}

const graphics::PassAttachment* graphics::TAAPass::GetDepthAttachment() const
{
	return nullptr;
}