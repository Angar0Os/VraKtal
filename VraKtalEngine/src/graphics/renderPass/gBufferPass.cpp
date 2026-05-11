#include <graphics/renderPass/gBufferPass.h>

#include <factory/materialFactory.h>

#include <core/gpu/descriptorSet.h>
#include <core/enum.h>
#include <loaders/shaderLoader.h>
#include <graphics/assets/mesh.h>

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
	CreateAttachments();
	CreateDescriptorSetLayout();
	CreateMaterialLayout();
	CreatePipeline();
	CreateDescriptorSets();
	CreateFallbackMaterial();
}

void graphics::GBufferPass::CreateAttachments()
{
	auto [width, height] = m_device.GetSwapchainExtent();

	const TextureFormat colorFormats[] = {
		TextureFormat::RGBA8_SRGB,
		TextureFormat::RGBA16_Float,
		TextureFormat::RG32_Float
	};

	m_colorAttachments.clear();
	m_colorAttachments.resize(3);

	for (int i = 0; i < 3; ++i)
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
		m_colorAttachments[i].texture = std::make_unique<Texture>(m_device, *m_colorAttachments[i].image);
	}

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
	m_depthAttachment.texture = std::make_unique<Texture>(m_device, *m_depthAttachment.image);
}

void graphics::GBufferPass::CreateDescriptorSetLayout()
{
	SDescriptorSetLayoutBinding uboBinding{
		.binding = 0,
		.descriptorType = EDescriptorType::UniformBuffer,
		.stageFlags = core::ShaderStage::Vertex | core::ShaderStage::Fragment
	};

	m_dsLayouts.clear();
	m_dsLayouts.push_back(std::make_unique<DescriptorSetLayout>(
		&m_device,
		SDescriptorSetLayoutCreateInfo{ .bindings = { uboBinding } }
	));
}

void graphics::GBufferPass::CreateMaterialLayout()
{
	SDescriptorSetLayoutBinding materialUBOBinding{
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

	SDescriptorSetLayoutBinding roughMetalBinding{
		.binding = 3,
		.descriptorType = EDescriptorType::CombinedImageSampler,
		.stageFlags = core::ShaderStage::Fragment
	};

	m_materialLayout = std::make_unique<DescriptorSetLayout>(
		&m_device,
		SDescriptorSetLayoutCreateInfo{
			.bindings = {
				materialUBOBinding,
				albedoBinding,
				normalBinding,
				roughMetalBinding
			}
		}
	);
}

void graphics::GBufferPass::CreateFallbackMaterial()
{
	m_fallbackMaterial = factory::MaterialFactory::CreateDefault(m_device, m_materialLayout.get());
}

void graphics::GBufferPass::CreatePipeline()
{
	auto shaderCode = loaders::ReadFile("../bin/assets/shaders/gbuffer.spv");

	SVertexInputBinding vertexBinding{
		.binding = 0,
		.stride = sizeof(graphics::Vertex),
		.inputRate = VertexInputRate::Vertex
	};

	std::vector<SVertexInputAttribute> vertexAttributes = {
		{0, 0, TextureFormat::RGB32_Float, offsetof(graphics::Vertex, position)},
		{1, 0, TextureFormat::RGB32_Float, offsetof(graphics::Vertex, normal)},
		{2, 0, TextureFormat::RG32_Float,  offsetof(graphics::Vertex, uv)}
	};

	std::vector<PushConstantRange> pushConstants = {
		{
			.stageFlags = static_cast<uint32_t>(ShaderStageFlags::Vertex),
			.offset = 0,
			.size = sizeof(GBufferPushConstants) 
		}
	};

	PipelineCreateInfo pipelineInfo{};
	pipelineInfo.shaderStages = {
		{ShaderStageFlags::Vertex,   shaderCode, "vertMain"},
		{ShaderStageFlags::Fragment, shaderCode, "fragMain"}
	};
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
	pipelineInfo.colorAttachmentFormats = { TextureFormat::RGBA8_SRGB, TextureFormat::RGBA16_Float, TextureFormat::RG32_Float };
	pipelineInfo.depthAttachmentFormat = TextureFormat::Depth32F;
	pipelineInfo.descriptorSetLayouts = { m_dsLayouts[0].get(), m_materialLayout.get() };
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
	cmd.BindDescriptorSets(m_pipeline.get(), m_descriptorSets[frameIndex].get(), frameIndex, 0);
}

void graphics::GBufferPass::Draw(CommandBuffer& cmd,
	const std::vector<ColorAttachmentDesc>& colorAttachments,
	const DepthAttachmentDesc& depthAttachment, uint32_t currentFrame)
{
	for (auto& ca : m_colorAttachments)
		cmd.TransitionImageLayout(ca.image.get(), ImageLayout::Undefined, ImageLayout::ColorAttachment, false);

	cmd.TransitionImageLayout(m_depthAttachment.image.get(), ImageLayout::Undefined, ImageLayout::DepthStencilAttachment, true);

	std::vector<CommandBuffer::RenderingAttachmentInfo> colorInfos;
	colorInfos.reserve(m_colorAttachments.size());
	for (const auto& ca : m_colorAttachments)
	{
		CommandBuffer::RenderingAttachmentInfo info{};
		info.image = ca.image.get();
		info.clear = true;
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

	UpdateDescriptorSets(currentFrame);
	BindDescriptorSets(cmd, currentFrame);

	if (m_meshInstances)
	{
		uint32_t prevFrame = (currentFrame + Device::s_FRAMES_IN_FLIGHT - 1) % Device::s_FRAMES_IN_FLIGHT;

		for (const auto& [mesh, transform] : *m_meshInstances)
		{
			if (!mesh->vertexBuffer || !mesh->indexBuffer)
				continue;

			cmd.BindVertexBuffer(mesh->vertexBuffer.get());
			cmd.BindIndexBuffer(mesh->indexBuffer.get());

			struct PushConstants {
				glm::mat4 model;
				glm::mat4 prevModel;
			};

			PushConstants pc{};
			pc.model = transform;

			auto it = m_prevModelTransforms[prevFrame].find(mesh);
			if (it != m_prevModelTransforms[prevFrame].end())
			{
				pc.prevModel = it->second;
			}
			else
			{
				pc.prevModel = transform;
			}

			cmd.PushConstants(
				m_pipeline.get(),
				static_cast<uint32_t>(core::ShaderStageFlags::Vertex),
				0, sizeof(PushConstants), &pc
			);

			for (const auto& submesh : mesh->subMeshes)
			{
				auto& mat = mesh->GetMaterial(submesh.materialIndex);

				cmd.BindDescriptorSets(
					m_pipeline.get(),
					mat.descriptorSet.get(),
					0,
					1
				);

				cmd.DrawIndexed(
					submesh.indexCount,
					1,
					submesh.firstIndex,
					submesh.vertexOffset,
					0
				);
			}

			m_prevModelTransforms[currentFrame][mesh] = transform;
		}
	}

	cmd.EndRendering();

	for (auto& ca : m_colorAttachments)
		cmd.TransitionImageLayout(ca.image.get(), ImageLayout::ColorAttachment, ImageLayout::ShaderReadOnly, false);

	cmd.TransitionImageLayout(m_depthAttachment.image.get(), ImageLayout::DepthStencilAttachment, ImageLayout::ShaderReadOnly, true);
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