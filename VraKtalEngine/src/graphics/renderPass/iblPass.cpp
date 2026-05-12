#include <graphics/renderPass/iblPass.h>

#include <core/gpu/descriptorSet.h>
#include <core/gpu/device.h>
#include <core/enum.h>
#include <loaders/shaderLoader.h>
#include <factory/materialFactory.h>

using namespace core;
using namespace core::gpu;

graphics::IBLPass::IBLPass(Device& device,
    const std::vector<std::unique_ptr<Buffer>>& uniformBuffers)
    : Pass("IBL")
    , m_device(device)
    , m_uniformBuffers(uniformBuffers)
{
    LoadEnvironmentMaps();
    Init(device);
}

void graphics::IBLPass::LoadEnvironmentMaps()
{
    m_envMap.image = factory::MaterialFactory::UploadHDRTexture(
        m_device, "assets/textures/skyboxes/citrus_1k.hdr");
    m_envMap.texture = std::make_unique<Texture>(m_device, *m_envMap.image);

    auto shaderCode = loaders::ReadFile("../bin/assets/shaders/irradiance_convolution.spv");

    SDescriptorSetLayoutBinding envMapBinding{
        .binding = 0,
        .descriptorType = EDescriptorType::CombinedImageSampler,
        .stageFlags = core::ShaderStage::Compute
    };
    SDescriptorSetLayoutBinding irradianceOutput{
        .binding = 1,
        .descriptorType = EDescriptorType::StorageImage,
        .stageFlags = core::ShaderStage::Compute
    };

    SDescriptorSetLayoutCreateInfo convLayoutInfo{
        .bindings = { envMapBinding, irradianceOutput }
    };
    auto convLayout = std::make_unique<DescriptorSetLayout>(&m_device, convLayoutInfo);

    PipelineCreateInfo convPipelineInfo{
        .shaderStages = { {ShaderStageFlags::Compute, shaderCode, "cs_main"} },
        .descriptorSetLayouts = { convLayout.get() }
    };
    auto convPipeline = std::make_unique<Pipeline>(&m_device, convPipelineInfo);

    SImageCreateInfo irradianceInfo{
        .width = 128,
        .height = 64,
        .mipLevels = 1,
        .format = TextureFormat::RGBA32_Float,
        .tiling = ImageTiling::Optimal,
        .usage = ImageUsage::Sampled | ImageUsage::Storage,
        .memoryProperties = EMemoryProperty::DeviceLocal,
        .samples = SampleCount::e1
    };
    m_irradianceMap.image = std::make_unique<Image>(&m_device, irradianceInfo);

    auto convDs = std::make_unique<DescriptorSet>(&m_device, convLayout.get());
    convDs->Bind(0, *m_envMap.texture);
    convDs->Bind(1, *m_irradianceMap.image);
    convDs->Update(m_device);

    SCommandBufferCreateInfo cbInfo{
        .device = &m_device,
        .level = ECommandBufferLevel::Primary,
        .count = 1,
        .singleTime = true
    };
    CommandBuffer cmd(&m_device, cbInfo);
    cmd.Begin(0);

    cmd.TransitionImageLayout(m_envMap.image.get(),
        ImageLayout::Undefined, ImageLayout::ShaderReadOnly, false);
    cmd.TransitionImageLayout(m_irradianceMap.image.get(),
        ImageLayout::Undefined, ImageLayout::General, false);

    cmd.BindComputePipeline(convPipeline.get());
    cmd.BindDescriptorSets(convPipeline.get(), convDs.get(), 0, 0);
    cmd.Dispatch(128 / 16, 64 / 16, 1);

    cmd.TransitionImageLayout(m_irradianceMap.image.get(),
        ImageLayout::General, ImageLayout::ShaderReadOnly, false);

    cmd.End(0);
    cmd.SubmitImmediate(&m_device);

    m_irradianceMap.texture = std::make_unique<Texture>(m_device, *m_irradianceMap.image);
}

void graphics::IBLPass::Init(Device& device)
{
    CreateAttachments();
    CreateDescriptorSetLayout();
    CreatePipeline();
    CreateDescriptorSets();
}

void graphics::IBLPass::CreateAttachments()
{
    auto [width, height] = m_device.GetSwapchainExtent();

    SImageCreateInfo info{
        .width = width,
        .height = height,
        .mipLevels = 1,
        .format = TextureFormat::RGBA16_Float,  
        .tiling = ImageTiling::Optimal,
        .usage = ImageUsage::ColorAttachment
                          | ImageUsage::Sampled          
                          | ImageUsage::TransferSrc,
        .memoryProperties = EMemoryProperty::DeviceLocal,
        .samples = SampleCount::e1
    };

    m_colorAttachments.resize(1);
    m_colorAttachments[0].image = std::make_unique<Image>(&m_device, info);
    m_colorAttachments[0].texture = std::make_unique<Texture>(m_device, *m_colorAttachments[0].image);
}

void graphics::IBLPass::CreateDescriptorSetLayout()
{
    const auto frag = core::ShaderStage::Fragment;

    SDescriptorSetLayoutCreateInfo layoutInfo{
        .bindings = {
            {.binding = 0, .descriptorType = EDescriptorType::UniformBuffer,          .stageFlags = frag },
            {.binding = 1, .descriptorType = EDescriptorType::CombinedImageSampler,   .stageFlags = frag },
            {.binding = 2, .descriptorType = EDescriptorType::CombinedImageSampler,   .stageFlags = frag },
            {.binding = 3, .descriptorType = EDescriptorType::CombinedImageSampler,   .stageFlags = frag },
            {.binding = 4, .descriptorType = EDescriptorType::AccelerationStructure,  .stageFlags = frag },
            {.binding = 5, .descriptorType = EDescriptorType::CombinedImageSampler,   .stageFlags = frag },
            {.binding = 6, .descriptorType = EDescriptorType::CombinedImageSampler,   .stageFlags = frag },
        }
    };

    m_dsLayouts.clear();
    m_dsLayouts.push_back(std::make_unique<DescriptorSetLayout>(&m_device, layoutInfo));
}

void graphics::IBLPass::CreatePipeline()
{
    auto shaderCode = loaders::ReadFile("../bin/assets/shaders/ibl.spv");

    PipelineCreateInfo info{};
    info.shaderStages = {
        { ShaderStageFlags::Vertex,   shaderCode, "vertMain" },
        { ShaderStageFlags::Fragment, shaderCode, "fragMain" }
    };
    info.vertexBindings = {};
    info.vertexAttributes = {};
    info.topology = PrimitiveTopology::TriangleList;
    info.polygonMode = PolygonMode::Fill;
    info.cullMode = CullMode::None;
    info.frontFace = FrontFace::Clockwise;
    info.depthTestEnable = false;
    info.depthWriteEnable = false;
    info.depthCompareOp = CompareOp::Always;
    info.blendEnable = false;
    info.samples = SampleCount::e1;
    info.colorAttachmentFormats = { TextureFormat::RGBA16_Float };
    info.depthAttachmentFormat = TextureFormat::Undefined;
    info.descriptorSetLayouts = { m_dsLayouts[0].get() };
    info.pushConstantRanges = {};
    info.dynamicStates = { DynamicState::Viewport, DynamicState::Scissor };

    m_pipeline = std::make_unique<Pipeline>(&m_device, info);
}

void graphics::IBLPass::CreateDescriptorSets()
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

void graphics::IBLPass::SetGBufferInputs(const std::vector<PassAttachment>& colorAttachments,
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
        m_descriptorSets[i]->Bind(6, *m_irradianceMap.texture);
        m_descriptorSets[i]->Update(m_device);
    }
}

void graphics::IBLPass::SetTLAS(AccelerationStructure* tlas)
{
    m_tlas = tlas;
}

void graphics::IBLPass::UpdateDescriptorSets(uint32_t frameIndex)
{
    if (!m_tlas) return;

    m_descriptorSets[frameIndex]->Bind(4, *m_tlas);
    m_descriptorSets[frameIndex]->Update(m_device);
}

void graphics::IBLPass::BindDescriptorSets(CommandBuffer& cmd, uint32_t frameIndex)
{
    cmd.BindDescriptorSets(
        m_pipeline.get(),
        m_descriptorSets[frameIndex].get(),
        frameIndex,
        0
    );
}

void graphics::IBLPass::Draw(CommandBuffer& cmd,
    const std::vector<ColorAttachmentDesc>&,
    const DepthAttachmentDesc&,
    uint32_t currentFrame)
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

    UpdateDescriptorSets(currentFrame);
    BindDescriptorSets(cmd, currentFrame);

    cmd.DrawIndexed(3, 1, 0, 0, 0);  

    cmd.EndRendering();

    cmd.TransitionImageLayout(
        m_colorAttachments[0].image.get(),
        ImageLayout::ColorAttachment,
        ImageLayout::ShaderReadOnly,
        false
    );
}

const std::vector<graphics::PassAttachment>& graphics::IBLPass::GetColorAttachments() const
{
    return m_colorAttachments;
}

const graphics::PassAttachment* graphics::IBLPass::GetDepthAttachment() const
{
    return nullptr;
}