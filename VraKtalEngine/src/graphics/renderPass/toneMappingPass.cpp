#include <graphics/renderPass/toneMappingPass.h>
#include <core/gpu/descriptorSet.h>
#include <core/enum.h>
#include <loaders/shaderLoader.h>

using namespace core;
using namespace core::gpu;

graphics::ToneMappingPass::ToneMappingPass(Device& device)
    : Pass("ToneMapping")
    , m_device(device)
{
    Init(device);
}

void graphics::ToneMappingPass::Init(Device& device)
{
    CreateAttachments();
    CreateDescriptorSetLayout();
    CreatePipeline();
    CreateDescriptorSets();
}

void graphics::ToneMappingPass::CreateAttachments()
{
    auto [width, height] = m_device.GetSwapchainExtent();

    SImageCreateInfo info{
        .width = width,
        .height = height,
        .mipLevels = 1,
        .format = TextureFormat::RGBA16_Float,
        .tiling = ImageTiling::Optimal,
        .usage = ImageUsage::Storage | ImageUsage::Sampled
                | ImageUsage::TransferSrc | ImageUsage::TransferDst,
        .memoryProperties = EMemoryProperty::DeviceLocal,
        .samples = SampleCount::e1
    };

    m_colorAttachments.resize(1);
    m_colorAttachments[0].image = std::make_unique<Image>(&m_device, info);
    m_colorAttachments[0].texture = std::make_unique<Texture>(m_device, *m_colorAttachments[0].image);
}

void graphics::ToneMappingPass::CreateDescriptorSetLayout()
{
    SDescriptorSetLayoutCreateInfo layoutInfo{
        .bindings = {
            {.binding = 0, .descriptorType = EDescriptorType::CombinedImageSampler, .stageFlags = core::ShaderStage::Compute },
            {.binding = 1, .descriptorType = EDescriptorType::StorageImage,         .stageFlags = core::ShaderStage::Compute },
        }
    };

    m_dsLayouts.clear();
    m_dsLayouts.push_back(std::make_unique<DescriptorSetLayout>(&m_device, layoutInfo));
}

void graphics::ToneMappingPass::CreatePipeline()
{
    auto shaderCode = loaders::ReadFile("../bin/assets/shaders/toneMapping.spv");

    PipelineCreateInfo pipelineInfo{
        .shaderStages = { { ShaderStageFlags::Compute, shaderCode, "cs_main" } },
        .descriptorSetLayouts = { m_dsLayouts[0].get() }
    };

    m_pipeline = std::make_unique<Pipeline>(&m_device, pipelineInfo);
}

void graphics::ToneMappingPass::CreateDescriptorSets()
{
    m_descriptorSets.clear();
    auto ds = std::make_unique<DescriptorSet>(&m_device, m_dsLayouts[0].get());
    m_descriptorSets.push_back(std::move(ds));
}

void graphics::ToneMappingPass::SetInput(const PassAttachment& input)
{
    m_input = &input;

    m_descriptorSets[0]->Bind(0, *m_input->texture);
    m_descriptorSets[0]->Bind(1, *m_colorAttachments[0].image);
    m_descriptorSets[0]->Update(m_device);
}

void graphics::ToneMappingPass::UpdateDescriptorSets(uint32_t) {}
void graphics::ToneMappingPass::BindDescriptorSets(CommandBuffer& cmd, uint32_t)
{
    cmd.BindDescriptorSets(m_pipeline.get(), m_descriptorSets[0].get(), 0, 0);
}

void graphics::ToneMappingPass::Draw(CommandBuffer& cmd,
    const std::vector<ColorAttachmentDesc>&,
    const DepthAttachmentDesc&,
    uint32_t)
{
    auto [width, height] = m_device.GetSwapchainExtent();

    cmd.TransitionImageLayout(
        m_colorAttachments[0].image.get(),
        ImageLayout::Undefined, ImageLayout::General, false
    );

    cmd.BindComputePipeline(m_pipeline.get());
    BindDescriptorSets(cmd, 0);
    cmd.Dispatch((width + 15) / 16, (height + 15) / 16, 1);

    cmd.TransitionImageLayout(
        m_colorAttachments[0].image.get(),
        ImageLayout::General, ImageLayout::ShaderReadOnly, false
    );
}

const std::vector<graphics::PassAttachment>& graphics::ToneMappingPass::GetColorAttachments() const
{
    return m_colorAttachments;
}

const graphics::PassAttachment* graphics::ToneMappingPass::GetDepthAttachment() const
{
    return nullptr;
}