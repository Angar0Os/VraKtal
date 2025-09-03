#include "../src/graphics/pbr/pbrPipeline.h"
#include "../src/graphics/loaders/shaderLoader.h"
#include "../src/core/gpu/pipeline_impl_vulkan.h"
#include "../src/core/gpu/gpuDevice_impl_glfw_vulkan.h"

using namespace graphics::pbr;
using namespace graphics::loaders;

PBRPipeline::PBRPipeline(GpuDeviceVulkan& device, VkRenderPass renderPass)
	: m_device(device)
{
	auto vertCode = ShaderLoader::ReadShaderFile("../bin/assets/shaders/pbr.vert.spv");
	auto fragCode = ShaderLoader::ReadShaderFile("../bin/assets/shaders/pbr.frag.spv");

	VkShaderModule vertShader = ShaderLoader::CreateShaderModule(device.Device(), vertCode);
	VkShaderModule fragShader = ShaderLoader::CreateShaderModule(device.Device(), fragCode);

	VkPipelineShaderStageCreateInfo vertStage{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertStage.module = vertShader;
	vertStage.pName = "main";

	VkPipelineShaderStageCreateInfo fragStage{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragStage.module = fragShader;
	fragStage.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertStage, fragStage };

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPipelineRasterizationStateCreateInfo rasterizer{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.lineWidth = 1.0f;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlending{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineViewportStateCreateInfo viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineDynamicStateCreateInfo dynamicState{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
    std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    dynamicState.dynamicStateCount = (uint32_t)dynamicStates.size();
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineDepthStencilStateCreateInfo depthStencil{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

    VkPipelineMultisampleStateCreateInfo multisampling{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    std::vector<VkDescriptorSetLayout> setLayouts = { m_globalSetLayout, m_materialSetLayout };
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    pipelineLayoutInfo.setLayoutCount = (uint32_t)setLayouts.size();
    pipelineLayoutInfo.pSetLayouts = setLayouts.data();
    vkCreatePipelineLayout(m_device.Device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout);

    VkPipelineVertexInputStateCreateInfo vertexInput{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

    VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInput;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    VkPipeline pipeline;

    if (vkCreateGraphicsPipelines(m_device.Device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create PBR graphics pipeline");
    }

    m_pipeline = new PipelineVulkan(m_device, pipeline, m_pipelineLayout);

    vkDestroyShaderModule(m_device.Device(), vertShader, nullptr);
    vkDestroyShaderModule(m_device.Device(), fragShader, nullptr);
}

PBRPipeline::~PBRPipeline()
{
	if (m_pipeline)
	{
		vkDestroyPipeline(m_device.Device(), m_pipeline, nullptr);
	}

	if (m_pipelineLayout)
	{
		vkDestroyPipelineLayout(m_device.Device(), m_pipelineLayout, nullptr);
	}

	if (m_globalSetLayout)
	{
		vkDestroyDescriptorSetLayout(m_device.Device(), m_globalSetLayout, nullptr);
	}

	if (m_materialSetLayout)
	{
		vkDestroyDescriptorSetLayout(m_device.Device(), m_materialSetLayout, nullptr);
	}
}