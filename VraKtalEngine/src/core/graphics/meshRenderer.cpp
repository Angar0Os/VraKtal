#include <core/graphics/meshRenderer.h>
#include <core/loaders/shaderLoader.h>
#include <core/graphics/resources/material.h>
#include <core/graphics/resources/mesh.h>

#include "../src/core/rhi/commandBuffer_impl_vulkan.h"
#include "../src/core/rhi/gpuDevice_impl_glfw_vulkan.h"

#include <stdexcept>
#include <array>

using namespace core::loaders;
using namespace core::graphics;
using namespace core::rhi::vulkan;

MeshRenderer::MeshRenderer(GpuDeviceVulkan& device)
    : m_device(device)
{
    CreatePipeline();
}

MeshRenderer::~MeshRenderer()
{
    VkDevice dev = m_device.Device();
    if (m_pipeline)         vkDestroyPipeline(dev, m_pipeline, nullptr);
    if (m_pipelineLayout)   vkDestroyPipelineLayout(dev, m_pipelineLayout, nullptr);
    DestroyDescriptors();
}

void MeshRenderer::SetMaterialDescriptorSets(const std::vector<VkDescriptorSet>& sets)
{
    m_materialDescriptorSets = sets;
}

void MeshRenderer::CreateDescriptorSetLayout()
{
    if (m_descriptorSetLayout) 
    {
        return;
    }

    VkDescriptorSetLayoutBinding binding{};
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    info.bindingCount = 1;
    info.pBindings = &binding;

    if (vkCreateDescriptorSetLayout(m_device.Device(), &info, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor set layout");
    }
}

void MeshRenderer::DestroyDescriptors() 
{
    VkDevice dev = m_device.Device();
    if (m_descriptorPool) 
    {
        vkDestroyDescriptorPool(dev, m_descriptorPool, nullptr);
        m_descriptorPool = VK_NULL_HANDLE;
    }
    if (m_descriptorSetLayout) 
    {
        vkDestroyDescriptorSetLayout(dev, m_descriptorSetLayout, nullptr);
        m_descriptorSetLayout = VK_NULL_HANDLE;
    }
}

void MeshRenderer::CreateDescriptorPool(uint32_t maxSets) 
{
    if (m_descriptorPool)
    {
        return;
    }

    if (maxSets == 0)
    {
        maxSets = 1;
    }

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = maxSets;

    VkDescriptorPoolCreateInfo info{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    info.poolSizeCount = 1;
    info.pPoolSizes = &poolSize;
    info.maxSets = maxSets;

    if (vkCreateDescriptorPool(m_device.Device(), &info, nullptr, &m_descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor pool");
    }
}

const VkDevice MeshRenderer::Device()
{ 
    return m_device.Device();
}

VkDescriptorSet MeshRenderer::CreateDescriptorSet(VkImageView view, VkSampler sampler) {
    if (!m_descriptorSetLayout)
    {
        throw std::runtime_error("Descriptor set layout not created");
    }
    if (!m_descriptorPool) 
    {
        throw std::runtime_error("Descriptor pool not created");
    }

    VkDescriptorSetAllocateInfo alloc{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    alloc.descriptorPool = m_descriptorPool;
    alloc.descriptorSetCount = 1;
    alloc.pSetLayouts = &m_descriptorSetLayout;

    VkDescriptorSet set;
    if (vkAllocateDescriptorSets(m_device.Device(), &alloc, &set) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate descriptor set");
    }

    VkDescriptorImageInfo imgInfo{};
    imgInfo.imageView = view;
    imgInfo.sampler = sampler;
    imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    write.dstSet = set;
    write.dstBinding = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = &imgInfo;

    vkUpdateDescriptorSets(m_device.Device(), 1, &write, 0, nullptr);
    return set;
}

void MeshRenderer::CreatePipeline()
{
    auto vertShaderCode = ShaderLoader::ReadShaderFile("../bin/assets/shaders/mesh.vert.spv");
    auto fragShaderCode = ShaderLoader::ReadShaderFile("../bin/assets/shaders/mesh.frag.spv");

    VkShaderModule vertShaderModule = ShaderLoader::CreateShaderModule(m_device.Device(), vertShaderCode);
    VkShaderModule fragShaderModule = ShaderLoader::CreateShaderModule(m_device.Device(), fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(resources::Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};
    
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(resources::Vertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(resources::Vertex, normal);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(resources::Vertex, uv);

    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(resources::Vertex, tangent);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = 1.0f;
    viewport.height = 1.0f;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = { 1, 1 };

    VkPipelineViewportStateCreateInfo viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | 
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(glm::mat4) * 3;

    CreateDescriptorSetLayout();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;

    if (vkCreatePipelineLayout(m_device.Device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout");
    }

    VkPipelineDepthStencilStateCreateInfo depthStencil{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkFormat colorFormat = m_device.SwapFormat();
    VkFormat depthFormat = m_device.DepthFormat();
    VkPipelineRenderingCreateInfo renderingInfo{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachmentFormats = &colorFormat;
    renderingInfo.depthAttachmentFormat = depthFormat;

    VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    pipelineInfo.pNext = &renderingInfo;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = VK_NULL_HANDLE; 
    pipelineInfo.pDepthStencilState = &depthStencil;

    if (vkCreateGraphicsPipelines(m_device.Device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create graphics pipeline");
    }

    ShaderLoader::DestroyShaderModule(m_device.Device(), fragShaderModule);
    ShaderLoader::DestroyShaderModule(m_device.Device(), vertShaderModule);
}

GpuMesh MeshRenderer::UploadMesh(const resources::Mesh& mesh)
{
    GpuMesh gpuMesh{};

    VkDeviceSize vertexBufferSize = mesh.vertices.size() * sizeof(resources::Vertex);
    VkDeviceSize indexBufferSize  = mesh.indices.size() * sizeof(uint32_t);

    {
        VkBufferCreateInfo vbInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        vbInfo.size  = vertexBufferSize;
        vbInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        if (vmaCreateBuffer(m_device.Allocator(), &vbInfo, &allocInfo,
                            &gpuMesh.vertexBuffer, &gpuMesh.vertexAlloc, nullptr) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create vertex buffer");
        }

        m_device.UploadToBuffer(gpuMesh.vertexBuffer, mesh.vertices.data(), vertexBufferSize);
    }

    {
        VkBufferCreateInfo ibInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        ibInfo.size  = indexBufferSize;
        ibInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        if (vmaCreateBuffer(m_device.Allocator(), &ibInfo, &allocInfo,
                            &gpuMesh.indexBuffer, &gpuMesh.indexAlloc, nullptr) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create index buffer");
        }

        m_device.UploadToBuffer(gpuMesh.indexBuffer, mesh.indices.data(), indexBufferSize);
    }

    gpuMesh.indexCount = static_cast<uint32_t>(mesh.indices.size());
    gpuMesh.materialIndex = mesh.materialIndex;
    return gpuMesh;
}

void MeshRenderer::DestroyMesh(GpuMesh& mesh)
{
    if (mesh.vertexBuffer)
    {
        vmaDestroyBuffer(m_device.Allocator(), mesh.vertexBuffer, mesh.vertexAlloc);
        mesh.vertexBuffer = VK_NULL_HANDLE;
    }
    if (mesh.indexBuffer)
    {
        vmaDestroyBuffer(m_device.Allocator(), mesh.indexBuffer, mesh.indexAlloc);
        mesh.indexBuffer = VK_NULL_HANDLE;
    }
}

void MeshRenderer::Draw(CommandBufferVulkan& cmd, const GpuMesh& mesh,
    const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection)
{
    VkCommandBuffer nativeCmd = cmd.GetNative();

    VkDeviceSize offsets[] = { 0 };
    VkBuffer vertexBuffers[] = { mesh.vertexBuffer };

    vkCmdBindPipeline(nativeCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

    if (mesh.materialIndex >= 0 && mesh.materialIndex < m_materialDescriptorSets.size())
    {
        vkCmdBindDescriptorSets(nativeCmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipelineLayout, 0, 1,
            &m_materialDescriptorSets[mesh.materialIndex], 0, nullptr);
    }

    struct PushConstants 
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    } pushConstants;

    pushConstants.model = model;
    pushConstants.view = view;
    pushConstants.proj = projection;

    vkCmdPushConstants(nativeCmd, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
        0, sizeof(PushConstants), &pushConstants);

    vkCmdBindVertexBuffers(nativeCmd, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(nativeCmd, mesh.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(nativeCmd, mesh.indexCount, 1, 0, 0, 0);
}