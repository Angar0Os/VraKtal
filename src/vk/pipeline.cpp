#include <vk/pipeline.h>
#include <vk/descriptors.h>
#include <vk/device.h>
#include <vk/commandBuffer.h>

#include <fstream>
#include <iostream>

using namespace vk;

bool vkutils::LoadShaderModule(const char* filePath, VkDevice device, VkShaderModule * outShaderModule)
{
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		return false;
	}

	size_t fileSize = (size_t)file.tellg();

	std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

	file.seekg(0);

	file.read((char*)buffer.data(), fileSize);

	file.close();

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pNext = nullptr;

	createInfo.codeSize = buffer.size() * sizeof(uint32_t);
	createInfo.pCode = buffer.data();

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		return false;
	}

	*outShaderModule = shaderModule;
	return true;
}

void VulkanPipeline::Clear()
{
	_inputAssembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	_rasterizer = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	_colorBlendAttachment = {};
	_multisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	_pipelineLayout = {};
	_depthStencil = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	_renderInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
	_shaderStages.clear();
}

VkPipeline VulkanPipeline::BuildPipeline(VkDevice device)
{
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pNext = nullptr;

	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.pNext = nullptr;

	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &_colorBlendAttachment;

	VkPipelineVertexInputStateCreateInfo _vertexInputInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

	VkGraphicsPipelineCreateInfo pipelineInfo = { .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

	pipelineInfo.pNext = &_renderInfo;

	pipelineInfo.stageCount = (uint32_t)_shaderStages.size();
	pipelineInfo.pStages = _shaderStages.data();
	pipelineInfo.pVertexInputState = &_vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &_inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &_rasterizer;
	pipelineInfo.pMultisampleState = &_multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDepthStencilState = &_depthStencil;
	pipelineInfo.layout = _pipelineLayout;

	VkDynamicState state[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamicInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicInfo.pDynamicStates = &state[0];
	dynamicInfo.dynamicStateCount = 2;

	pipelineInfo.pDynamicState = &dynamicInfo;

	VkPipeline newPipeline = VK_NULL_HANDLE;
	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS)
	{
		std::cout << "Failed to create pipeline" << std::endl;
		return VK_NULL_HANDLE;
	}
	else
	{
		return newPipeline;
	}
}

void VulkanPipeline::SetShaders(VkShaderModule vertexShader, VkShaderModule fragmentShader)
{
	_shaderStages.clear();

	_shaderStages.push_back(PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertexShader));
	_shaderStages.push_back(PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader));
}

void VulkanPipeline::SetInputTopology(VkPrimitiveTopology topology)
{
	_inputAssembly.topology = topology;
	_inputAssembly.primitiveRestartEnable = VK_FALSE;
}

void VulkanPipeline::SetPolygonMode(VkPolygonMode mode)
{
	_rasterizer.polygonMode = mode;
	_rasterizer.lineWidth = 1.f;
}

void VulkanPipeline::SetCullMode(VkCullModeFlags cullMode, VkFrontFace frontFace)
{
	_rasterizer.cullMode = cullMode;
	_rasterizer.frontFace = frontFace;
}

void VulkanPipeline::SetMultisamplingNone()
{
	_multisampling.sampleShadingEnable = VK_FALSE;
	_multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	_multisampling.minSampleShading = 1.0f;
	_multisampling.pSampleMask = nullptr;
	_multisampling.alphaToCoverageEnable = VK_FALSE;
	_multisampling.alphaToOneEnable = VK_FALSE;
}

void VulkanPipeline::DisableBlending()
{
	_colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	_colorBlendAttachment.blendEnable = VK_FALSE;
}

void VulkanPipeline::SetColorAttachmentFormat(VkFormat format)
{
	_colorAttachmentFormat = format;
	_renderInfo.colorAttachmentCount = 1;
	_renderInfo.pColorAttachmentFormats = &_colorAttachmentFormat;
}

void VulkanPipeline::SetDepthFormat(VkFormat format)
{
	_renderInfo.depthAttachmentFormat = format;
}

void VulkanPipeline::DisableDepthtest()
{
	_depthStencil.depthTestEnable = VK_FALSE;
	_depthStencil.depthWriteEnable = VK_FALSE;
	_depthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
	_depthStencil.depthBoundsTestEnable = VK_FALSE;
	_depthStencil.stencilTestEnable = VK_FALSE;
	_depthStencil.front = {};
	_depthStencil.back = {};
	_depthStencil.minDepthBounds = 0.f;
	_depthStencil.maxDepthBounds = 1.f;
}


void VulkanPipeline::EnableDepthtest(bool depthWriteEnable, VkCompareOp op)
{
	_depthStencil.depthTestEnable = VK_TRUE;
	_depthStencil.depthWriteEnable = depthWriteEnable;
	_depthStencil.depthCompareOp = op;
	_depthStencil.depthBoundsTestEnable = VK_FALSE;
	_depthStencil.stencilTestEnable = VK_FALSE;
	_depthStencil.front = {};
	_depthStencil.back = {};
	_depthStencil.minDepthBounds = 0.f;
	_depthStencil.maxDepthBounds = 1.f;
}

void VulkanPipeline::EnableBlendingAdditive()
{
	_colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	_colorBlendAttachment.blendEnable = VK_TRUE;
	_colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	_colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
	_colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	_colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	_colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	_colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
}

void VulkanPipeline::EnableBlendingAlphablend()
{
	_colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	_colorBlendAttachment.blendEnable = VK_TRUE;
	_colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	_colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	_colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	_colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	_colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	_colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
}

VkPipelineShaderStageCreateInfo VulkanPipeline::PipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shaderModule)
{
	VkPipelineShaderStageCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	info.pNext = nullptr;
	info.pName = "main";

	info.stage = stage;
	info.module = shaderModule;
	return info;
}

VkPipelineLayoutCreateInfo VulkanPipeline::PipelineLayoutCreateInfo()
{
	VkPipelineLayoutCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	info.pNext = nullptr;

	info.flags = 0;
	info.setLayoutCount = 0;
	info.pSetLayouts = nullptr;
	info.pushConstantRangeCount = 0;
	info.pPushConstantRanges = nullptr;
	return info;
}

void VulkanPipeline::InitBackgroundPipelines(VulkanDescriptor* descriptor, VulkanDevice device, VulkanCommandBuffer* commandBuffer)
{
	VkPipelineLayoutCreateInfo computeLayout{};
	computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	computeLayout.pNext = nullptr;
	computeLayout.pSetLayouts = descriptor->GetDrawImageLayout();
	computeLayout.setLayoutCount = 1;

	VkPushConstantRange pushConstant{};
	pushConstant.offset = 0;
	pushConstant.size = sizeof(ComputePushConstants);
	pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	computeLayout.pPushConstantRanges = &pushConstant;
	computeLayout.pushConstantRangeCount = 1;

	vkCreatePipelineLayout(device.GetVkDevice(), &computeLayout, nullptr, &_gradientPipelineLayout);

	VkShaderModule gradientShader;
	if (!vkutils::LoadShaderModule("assets/shaders/gradient_color.comp.spv", device.GetVkDevice(), &gradientShader))
	{
		std::cout << "Error when building the compute shader" << std::endl;
	}

	VkShaderModule skyShader;
	if (!vkutils::LoadShaderModule("assets/shaders/sky.comp.spv", device.GetVkDevice(), &skyShader))
	{
		std::cout << "Error when building the compute shader" << std::endl;
	}

	VkPipelineShaderStageCreateInfo stageinfo{};
	stageinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stageinfo.pNext = nullptr;
	stageinfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	stageinfo.module = gradientShader;
	stageinfo.pName = "main";

	VkComputePipelineCreateInfo computePipelineCreateInfo{};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.pNext = nullptr;
	computePipelineCreateInfo.layout = _gradientPipelineLayout;
	computePipelineCreateInfo.stage = stageinfo;

	ComputeEffect gradient;
	gradient.layout = _gradientPipelineLayout;
	gradient.name = "gradient";
	gradient.data = {};

	gradient.data.data1 = glm::vec4(1, 0, 0, 1);
	gradient.data.data2 = glm::vec4(0, 0, 1, 1);

	vkCreateComputePipelines(device.GetVkDevice(), VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &gradient.pipeline);

	computePipelineCreateInfo.stage.module = skyShader;

	ComputeEffect sky;
	sky.layout = _gradientPipelineLayout;
	sky.name = "sky";
	sky.data = {};
	sky.data.data1 = glm::vec4(0.1, 0.2, 0.4, 0.97);

	vkCreateComputePipelines(device.GetVkDevice(), VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &sky.pipeline);

	backgroundEffects.push_back(gradient);
	backgroundEffects.push_back(sky);

	vkDestroyShaderModule(device.GetVkDevice(), gradientShader, nullptr);
	vkDestroyShaderModule(device.GetVkDevice(), skyShader, nullptr);

	commandBuffer->GetDeletionQueue().push_function([=]() {
		vkDestroyPipelineLayout(device.GetVkDevice(), _gradientPipelineLayout, nullptr);
		vkDestroyPipeline(device.GetVkDevice(), sky.pipeline, nullptr);
		vkDestroyPipeline(device.GetVkDevice(), gradient.pipeline, nullptr);
		});
}
