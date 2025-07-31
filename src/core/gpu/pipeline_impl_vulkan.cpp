#include "pipeline_impl_vulkan.h"
#include "../renderContext_impl_glfw_vulkan.h"
#include "../../core/gpu/descriptor_impl_vulkan.h"

#include <fstream>
#include <iostream>

using namespace core::rhi::gpu;


namespace core::rhi::gpu
{
	Pipeline::~Pipeline() = default;
}

bool utils::LoadShaderModule(const char* filePath, VkDevice device, VkShaderModule* outShaderModule)
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

void Pipeline::Internal::InitBackgroundPipelines(RenderContext& rCtx)
{
	VkPipelineLayoutCreateInfo computeLayout{};
	computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	computeLayout.pNext = nullptr;
	computeLayout.pSetLayouts = &rCtx.GetInternal().descriptor->GetInternal().drawImageDescriptorLayout;
	computeLayout.setLayoutCount = 1;

	VkPushConstantRange pushConstant{};
	pushConstant.offset = 0;
	pushConstant.size = sizeof(ComputePushConstants);
	pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	computeLayout.pPushConstantRanges = &pushConstant;
	computeLayout.pushConstantRangeCount = 1;

	vkCreatePipelineLayout(rCtx.GetInternal().device, &computeLayout, nullptr, &gradientPipelineLayout);

	VkShaderModule gradientShader;
	if (!utils::LoadShaderModule("assets/shader/sky.comp.spv", rCtx.GetInternal().device, &gradientShader))
	{
		std::cout << "Error when building the compute shader" << std::endl;
	}

	VkShaderModule skyShader;
	if (!utils::LoadShaderModule("assets/shaders/sky.comp.spv", rCtx.GetInternal().device, &skyShader))
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
	computePipelineCreateInfo.layout = gradientPipelineLayout;
	computePipelineCreateInfo.stage = stageinfo;

	ComputeEffect gradient;
	gradient.layout = gradientPipelineLayout;
	gradient.name = "gradient";
	gradient.data = {};

	gradient.data.data1 = glm::vec4(1, 0, 0, 1);
	gradient.data.data2 = glm::vec4(0, 0, 1, 1);

	vkCreateComputePipelines(rCtx.GetInternal().device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &gradient.pipeline);

	computePipelineCreateInfo.stage.module = skyShader;

	ComputeEffect sky;
	sky.layout = gradientPipelineLayout;
	sky.name = "sky";
	sky.data = {};
	sky.data.data1 = glm::vec4(0.1, 0.2, 0.4, 0.97);

	vkCreateComputePipelines(rCtx.GetInternal().device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &sky.pipeline);

	backgroundEffects.push_back(gradient);
	backgroundEffects.push_back(sky);

	vkDestroyShaderModule(rCtx.GetInternal().device, gradientShader, nullptr);
	vkDestroyShaderModule(rCtx.GetInternal().device, skyShader, nullptr);

	VkDevice deviceHandle = rCtx.GetInternal().device;

	rCtx.GetInternal().commandBuffer->GetInternal().mainDeletionQueue.PushFunction([=]() {
		vkDestroyPipelineLayout(deviceHandle, gradientPipelineLayout, nullptr);
		vkDestroyPipeline(deviceHandle, sky.pipeline, nullptr);
		vkDestroyPipeline(deviceHandle, gradient.pipeline, nullptr);
		});
}

VkPipelineShaderStageCreateInfo Pipeline::Internal::PipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shaderModule)
{
	VkPipelineShaderStageCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	info.pNext = nullptr;
	info.pName = "main";

	info.stage = stage;
	info.module = shaderModule;
	return info;
}

void Pipeline::Internal::Clear()
{
	InputAssembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	Rasterizer = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	ColorBlendAttachment = {};
	Multisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	PipelineLayout = {};
	DepthStencil = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	RenderInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
	ShaderStages.clear();
}

VkPipeline Pipeline::Internal::BuildPipeline(VkDevice device)
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
	colorBlending.pAttachments = &ColorBlendAttachment;

	VkPipelineVertexInputStateCreateInfo _vertexInputInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

	VkGraphicsPipelineCreateInfo pipelineInfo = { .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

	pipelineInfo.pNext = &RenderInfo;

	pipelineInfo.stageCount = (uint32_t)ShaderStages.size();
	pipelineInfo.pStages = ShaderStages.data();
	pipelineInfo.pVertexInputState = &_vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &InputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &Rasterizer;
	pipelineInfo.pMultisampleState = &Multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDepthStencilState = &DepthStencil;
	pipelineInfo.layout = PipelineLayout;

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

void Pipeline::Internal::SetShaders(VkShaderModule vertexShader, VkShaderModule fragmentShader)
{
	ShaderStages.clear();

	ShaderStages.push_back(PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertexShader));
	ShaderStages.push_back(PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader));
}

void Pipeline::Internal::SetInputTopology(VkPrimitiveTopology topology)
{
	InputAssembly.topology = topology;
	InputAssembly.primitiveRestartEnable = VK_FALSE;
}

void Pipeline::Internal::SetPolygonMode(VkPolygonMode mode)
{
	Rasterizer.polygonMode = mode;
	Rasterizer.lineWidth = 1.f;
}

void Pipeline::Internal::SetCullMode(VkCullModeFlags cullMode, VkFrontFace frontFace)
{
	Rasterizer.cullMode = cullMode;
	Rasterizer.frontFace = frontFace;
}

void Pipeline::Internal::SetMultisamplingNone()
{
	Multisampling.sampleShadingEnable = VK_FALSE;
	Multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	Multisampling.minSampleShading = 1.0f;
	Multisampling.pSampleMask = nullptr;
	Multisampling.alphaToCoverageEnable = VK_FALSE;
	Multisampling.alphaToOneEnable = VK_FALSE;
}

void Pipeline::Internal::DisableBlending()
{
	ColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	ColorBlendAttachment.blendEnable = VK_FALSE;
}

void Pipeline::Internal::SetColorAttachmentFormat(VkFormat format)
{
	ColorAttachmentFormat = format;
	RenderInfo.colorAttachmentCount = 1;
	RenderInfo.pColorAttachmentFormats = &ColorAttachmentFormat;
}

void Pipeline::Internal::SetDepthFormat(VkFormat format)
{
	RenderInfo.depthAttachmentFormat = format;
}

void Pipeline::Internal::DisableDepthtest()
{
	DepthStencil.depthTestEnable = VK_FALSE;
	DepthStencil.depthWriteEnable = VK_FALSE;
	DepthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
	DepthStencil.depthBoundsTestEnable = VK_FALSE;
	DepthStencil.stencilTestEnable = VK_FALSE;
	DepthStencil.front = {};
	DepthStencil.back = {};
	DepthStencil.minDepthBounds = 0.f;
	DepthStencil.maxDepthBounds = 1.f;
}


void Pipeline::Internal::EnableDepthtest(bool depthWriteEnable, VkCompareOp op)
{
	DepthStencil.depthTestEnable = VK_TRUE;
	DepthStencil.depthWriteEnable = depthWriteEnable;
	DepthStencil.depthCompareOp = op;
	DepthStencil.depthBoundsTestEnable = VK_FALSE;
	DepthStencil.stencilTestEnable = VK_FALSE;
	DepthStencil.front = {};
	DepthStencil.back = {};
	DepthStencil.minDepthBounds = 0.f;
	DepthStencil.maxDepthBounds = 1.f;
}

void Pipeline::Internal::EnableBlendingAdditive()
{
	ColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	ColorBlendAttachment.blendEnable = VK_TRUE;
	ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
	ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
}

void Pipeline::Internal::EnableBlendingAlphablend()
{
	ColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	ColorBlendAttachment.blendEnable = VK_TRUE;
	ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
}

VkPipelineLayoutCreateInfo Pipeline::Internal::PipelineLayoutCreateInfo()
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