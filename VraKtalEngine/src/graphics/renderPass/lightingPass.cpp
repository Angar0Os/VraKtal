#include <graphics/renderPass/lightingPass.h>

#include <core/gpu/descriptorSet.h>
#include <core/enum.h>
#include <loaders/shaderLoader.h>

using namespace core;
using namespace core::gpu;

graphics::LightingPass::LightingPass(Device& device,
	const std::vector<std::unique_ptr<Buffer>>& uniformBuffers)
	: Pass("Lighting")
	, m_device(device)
	, m_uniformBuffers(uniformBuffers)
{
	m_envMap.image = loaders::MaterialLoader::UploadHDRTexture(m_device, "assets/textures/skyboxes/citrus_1k.hdr");
	m_envMap.texture = std::make_unique<Texture>(m_device, *m_envMap.image);
	Init(device);
}

void graphics::LightingPass::Init(Device& device)
{
	CreateAttachments();
	CreateDescriptorSetLayout();
	CreatePipeline();
	CreateDescriptorSets();
}

void graphics::LightingPass::CreateAttachments()
{
	auto [width, height] = m_device.GetSwapchainExtent();

	SImageCreateInfo info{
		.width = width,
		.height = height,
		.mipLevels = 1,
		.format = TextureFormat::RGBA8_SRGB,
		.tiling = ImageTiling::Optimal,
		.usage = ImageUsage::ColorAttachment | ImageUsage::TransferSrc,
		.memoryProperties = EMemoryProperty::DeviceLocal,
		.samples = SampleCount::e1
	};

	m_colorAttachments.resize(1);
	m_colorAttachments[0].image = std::make_unique<Image>(&m_device, info);
	m_colorAttachments[0].texture = std::make_unique<Texture>(m_device, *m_colorAttachments[0].image);
}

void graphics::LightingPass::CreateDescriptorSetLayout()
{
	SDescriptorSetLayoutBinding uboBinding{
		.binding = 0,
		.descriptorType = EDescriptorType::UniformBuffer,
		.stageFlags = core::ShaderStage::Fragment
	};
	SDescriptorSetLayoutBinding albedoBinding{
		.binding = 1,
		.descriptorType = EDescriptorType::CombinedImageSampler,
		.stageFlags = core::ShaderStage::Fragment
	};
	SDescriptorSetLayoutBinding normalBinding{
		.binding = 2,
		.descriptorType = EDescriptorType::CombinedImageSampler,
		.stageFlags = core::ShaderStage::Fragment
	};
	SDescriptorSetLayoutBinding depthBinding{
		.binding = 3,
		.descriptorType = EDescriptorType::CombinedImageSampler,
		.stageFlags = core::ShaderStage::Fragment
	};
	SDescriptorSetLayoutBinding tlasBinding{
		.binding = 4,
		.descriptorType = EDescriptorType::AccelerationStructure,
		.stageFlags = core::ShaderStage::Fragment
	};
	SDescriptorSetLayoutBinding envMapBinding{
		.binding = 5,
		.descriptorType = EDescriptorType::CombinedImageSampler,
		.stageFlags = core::ShaderStage::Fragment
	};

	SDescriptorSetLayoutCreateInfo layoutInfo{
		.bindings = { uboBinding, albedoBinding, normalBinding, depthBinding, tlasBinding, envMapBinding }
	};

	m_dsLayouts.clear();
	m_dsLayouts.push_back(std::make_unique<DescriptorSetLayout>(&m_device, layoutInfo));
}

void graphics::LightingPass::CreatePipeline()
{
	auto shaderCode = loaders::ReadFile("../bin/assets/shaders/lighting.spv");

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
	pipelineInfo.colorAttachmentFormats = { TextureFormat::RGBA8_SRGB };
	pipelineInfo.depthAttachmentFormat = TextureFormat::Undefined;
	pipelineInfo.descriptorSetLayouts = { m_dsLayouts[0].get() };
	pipelineInfo.pushConstantRanges = {};
	pipelineInfo.dynamicStates = { DynamicState::Viewport, DynamicState::Scissor };

	m_pipeline = std::make_unique<Pipeline>(&m_device, pipelineInfo);
}

void graphics::LightingPass::CreateDescriptorSets()
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

void graphics::LightingPass::SetGBufferInputs(const std::vector<PassAttachment>& colorAttachments,
	const PassAttachment& depthAttachment)
{
	m_gbufferColor = &colorAttachments;
	m_gbufferDepth = &depthAttachment;

	for (uint32_t i = 0; i < Device::s_FRAMES_IN_FLIGHT; ++i)
	{
		m_descriptorSets[i]->Bind(1, *colorAttachments[0].texture);
		m_descriptorSets[i]->Bind(2, *colorAttachments[1].texture);
		m_descriptorSets[i]->Bind(3, *depthAttachment.texture);
		m_descriptorSets[i]->Bind(5, *m_envMap.texture);
		m_descriptorSets[i]->Update(m_device);
	}
}

void graphics::LightingPass::SetTLAS(AccelerationStructure* tlas)
{
	m_tlas = tlas;
}

void graphics::LightingPass::UpdateDescriptorSets(uint32_t frameIndex)
{
	if (!m_tlas) return;

	m_descriptorSets[frameIndex]->Bind(4, *m_tlas);
	m_descriptorSets[frameIndex]->Update(m_device);
}

void graphics::LightingPass::BindDescriptorSets(CommandBuffer& cmd, uint32_t frameIndex)
{
	cmd.BindDescriptorSets(
		m_pipeline.get(),
		m_descriptorSets[frameIndex].get(),
		frameIndex,
		0
	);
}

void graphics::LightingPass::Draw(CommandBuffer& cmd,
	const std::vector<ColorAttachmentDesc>& colorAttachments,
	const DepthAttachmentDesc& depthAttachment)
{
	cmd.TransitionImageLayout(
		m_colorAttachments[0].image.get(),
		ImageLayout::Undefined,
		ImageLayout::ColorAttachment,
		false
	);

	CommandBuffer::RenderingAttachmentInfo colorInfo{};
	colorInfo.image = m_colorAttachments[0].image.get();
	colorInfo.clear = true;
	colorInfo.clearR = 0.0f;
	colorInfo.clearG = 0.0f;
	colorInfo.clearB = 0.0f;
	colorInfo.clearA = 1.0f;

	cmd.BeginRendering(&m_device, { colorInfo }, {});

	cmd.BindPipeline(m_pipeline.get());
	cmd.SetViewport(0.0f, 0.0f, &m_device);
	cmd.SetScissor(0, 0, &m_device);

	cmd.DrawIndexed(3, 1, 0, 0, 0);

	cmd.EndRendering();

	cmd.TransitionImageLayout(
		m_colorAttachments[0].image.get(),
		ImageLayout::ColorAttachment,
		ImageLayout::TransferSrc,
		false
	);
}

const std::vector<graphics::PassAttachment>& graphics::LightingPass::GetColorAttachments() const
{
	return m_colorAttachments;
}

const graphics::PassAttachment* graphics::LightingPass::GetDepthAttachment() const
{
	return nullptr;
}