#include "../src/core/gpu/vulkan/pipeline_impl.h"
#include "../src/core/gpu_detail/converters.h"
#include <core/gpu/descriptorSetLayout.h>

#include <stdexcept>

core::gpu::Pipeline::Impl::Impl(core::gpu::Pipeline& p, vk::raii::Device& dev, const PipelineCreateInfo& info)
    : parent(p), device(dev), pipelineLayout(nullptr), pipeline(nullptr)
{
    std::vector<vk::raii::ShaderModule> shaderModules;
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStageInfos;

    for (const auto& stage : info.shaderStages)
    {
        vk::ShaderModuleCreateInfo moduleInfo{};
        moduleInfo.codeSize = stage.code.size();
        moduleInfo.pCode = reinterpret_cast<const uint32_t*>(stage.code.data());

        shaderModules.emplace_back(device, moduleInfo);

        vk::PipelineShaderStageCreateInfo shaderStageInfo{};
        shaderStageInfo.stage = core::gpu_detail::ToVulkan(stage.stage);
        shaderStageInfo.module = *shaderModules.back();
        shaderStageInfo.pName = stage.entryPoint.c_str();

        shaderStageInfos.push_back(shaderStageInfo);
    }

    std::vector<vk::VertexInputBindingDescription> vkBindings;
    for (const auto& binding : info.vertexBindings)
    {
        vkBindings.push_back({
            binding.binding,
            binding.stride,
            core::gpu_detail::ToVulkan(binding.inputRate)
            });
    }

    std::vector<vk::VertexInputAttributeDescription> vkAttributes;
    for (const auto& attr : info.vertexAttributes)
    {
        vkAttributes.push_back({
            attr.location,
            attr.binding,
            core::gpu_detail::ToVulkan(attr.format),
            attr.offset
            });
    }

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vkBindings.size());
    vertexInputInfo.pVertexBindingDescriptions = vkBindings.data();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vkAttributes.size());
    vertexInputInfo.pVertexAttributeDescriptions = vkAttributes.data();

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.topology = core::gpu_detail::ToVulkan(info.topology);
    inputAssembly.primitiveRestartEnable = vk::False;

    vk::PipelineViewportStateCreateInfo viewportState{};
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.depthClampEnable = vk::False;
    rasterizer.rasterizerDiscardEnable = vk::False;
    rasterizer.polygonMode = core::gpu_detail::ToVulkan(info.polygonMode);
    rasterizer.cullMode = core::gpu_detail::ToVulkan(info.cullMode);
    rasterizer.frontFace = core::gpu_detail::ToVulkan(info.frontFace);
    rasterizer.depthBiasEnable = vk::False;
    rasterizer.lineWidth = 1.0f;

    vk::PipelineMultisampleStateCreateInfo multisampling{};
    multisampling.rasterizationSamples = core::gpu_detail::ToVulkan(info.samples);
    multisampling.sampleShadingEnable = vk::False;

    vk::PipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.depthTestEnable = info.depthTestEnable ? vk::True : vk::False;
    depthStencil.depthWriteEnable = info.depthWriteEnable ? vk::True : vk::False;
    depthStencil.depthCompareOp = core::gpu_detail::ToVulkan(info.depthCompareOp);
    depthStencil.depthBoundsTestEnable = vk::False;
    depthStencil.stencilTestEnable = vk::False;

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.blendEnable = info.blendEnable ? vk::True : vk::False;
    colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

    vk::PipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.logicOpEnable = vk::False;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    std::vector<vk::DynamicState> vkDynamicStates;
    for (const auto& state : info.dynamicStates)
    {
        vkDynamicStates.push_back(core::gpu_detail::ToVulkan(state));
    }

    vk::PipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.dynamicStateCount = static_cast<uint32_t>(vkDynamicStates.size());
    dynamicState.pDynamicStates = vkDynamicStates.data();

    std::vector<vk::DescriptorSetLayout> vkLayouts;
    for (auto* layout : info.descriptorSetLayouts)
    {
        VkDescriptorSetLayout vkLayoutHandle = reinterpret_cast<VkDescriptorSetLayout>(layout->GetHandle());
        vkLayouts.push_back(vk::DescriptorSetLayout(vkLayoutHandle));
    }

    vk::PipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.setLayoutCount = static_cast<uint32_t>(vkLayouts.size());
    layoutInfo.pSetLayouts = vkLayouts.data();

    pipelineLayout = vk::raii::PipelineLayout(device, layoutInfo);

    std::vector<vk::Format> vkColorFormats;
    for (const auto& format : info.colorAttachmentFormats)
    {
        vkColorFormats.push_back(core::gpu_detail::ToVulkan(format));
    }

    vk::PipelineRenderingCreateInfo renderingInfo{};
    renderingInfo.colorAttachmentCount = static_cast<uint32_t>(vkColorFormats.size());
    renderingInfo.pColorAttachmentFormats = vkColorFormats.data();
    renderingInfo.depthAttachmentFormat = core::gpu_detail::ToVulkan(info.depthAttachmentFormat);

    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.pNext = &renderingInfo;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStageInfos.size());
    pipelineInfo.pStages = shaderStageInfos.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = *pipelineLayout;
    pipelineInfo.renderPass = nullptr;

    pipeline = vk::raii::Pipeline(device, nullptr, pipelineInfo);
}

core::gpu::Pipeline::Impl::~Impl() = default;

vk::raii::Pipeline& core::gpu::Pipeline::Impl::GetPipeline()
{
    return pipeline;
}

const vk::raii::Pipeline& core::gpu::Pipeline::Impl::GetPipeline() const
{
    return pipeline;
}

vk::raii::PipelineLayout& core::gpu::Pipeline::Impl::GetPipelineLayout()
{
    return pipelineLayout;
}

const vk::raii::PipelineLayout& core::gpu::Pipeline::Impl::GetPipelineLayout() const
{
    return pipelineLayout;
}

core::gpu::Pipeline::Pipeline(void* device, const PipelineCreateInfo& info)
{
    auto& vkDevice = *static_cast<vk::raii::Device*>(device);
    m_impl = std::make_unique<Impl>(*this, vkDevice, info);
}

core::gpu::Pipeline::~Pipeline() = default;

core::gpu::Pipeline::Pipeline(Pipeline&&) noexcept = default;
core::gpu::Pipeline& core::gpu::Pipeline::operator=(Pipeline&&) noexcept = default;

void* core::gpu::Pipeline::GetHandle() const
{
    return static_cast<void*>(const_cast<vk::Pipeline*>(&(*m_impl->GetPipeline())));
}

void* core::gpu::Pipeline::GetLayoutHandle() const
{
    return static_cast<void*>(const_cast<vk::PipelineLayout*>(&(*m_impl->GetPipelineLayout())));
}

core::gpu::Pipeline::Impl& core::gpu::Pipeline::GetImpl()
{
    return *m_impl;
}

const core::gpu::Pipeline::Impl& core::gpu::Pipeline::GetImpl() const
{
    return *m_impl;
}