#ifndef VRAKTAL_CORE_GPU_PIPELINE_IMPL_VULKAN_H
#define VRAKTAL_CORE_GPU_PIPELINE_IMPL_VULKAN_H
#pragma once

#include <core/gpu/pipeline.h>
#include <core/renderContext.h>

#include <vulkan/vulkan.h>
#include <vector>

#include <glm/glm.hpp>

struct ComputePushConstants
{
	glm::vec4 data1;
	glm::vec4 data2;
	glm::vec4 data3;
	glm::vec4 data4;
};

struct ComputeEffect
{
	const char* name;

	VkPipeline pipeline;
	VkPipelineLayout layout;

	ComputePushConstants data;
};

namespace utils
{
	bool LoadShaderModule(const char* filePath, VkDevice device, VkShaderModule* outShaderModule);

	typedef struct VkGraphicsPipelineCreateInfo
	{
		VkStructureType									sType;
		const void* pNext;
		VkPipelineCreateFlags							flags;
		uint32_t										stageCount;
		const VkPipelineShaderStageCreateInfo*			pStages;
		const VkPipelineVertexInputStateCreateInfo*		pVertexInputState;
		const VkPipelineInputAssemblyStateCreateInfo*	pInputAssemblyState;
		const VkPipelineTessellationStateCreateInfo*	pTessellationState;
		const VkPipelineViewportStateCreateInfo*		pViewportState;
		const VkPipelineRasterizationStateCreateInfo*	pRasterizationState;
		const VkPipelineMultisampleStateCreateInfo*		pMultisampleState;
		const VkPipelineDepthStencilStateCreateInfo*	pDepthStencilState;
		const VkPipelineColorBlendStateCreateInfo*		pColorBlendState;
		const VkPipelineDynamicStateCreateInfo*			pDynamicState;
		VkPipelineLayout                                layout;
		VkRenderPass                                    renderPass;
		uint32_t                                        subpass;
		VkPipeline                                      basePipelineHandle;
		int32_t											basePipelineIndex;
	}VkGraphicsPipelineCreateInfo;
}

struct core::rhi::gpu::Pipeline::Internal
{
	Internal() { Clear(); };

	void Clear();
	void InitBackgroundPipelines(RenderContext& rCtx);
	void SetShaders(VkShaderModule vertexShader, VkShaderModule fragmentShader);
	void SetInputTopology(VkPrimitiveTopology topology);
	void SetPolygonMode(VkPolygonMode mode);
	void SetCullMode(VkCullModeFlags cullMode, VkFrontFace frontFace);
	void SetMultisamplingNone();
	void DisableBlending();
	void SetColorAttachmentFormat(VkFormat format);
	void SetDepthFormat(VkFormat format);
	void DisableDepthtest();
	void EnableDepthtest(bool depthWriteEnable, VkCompareOp op);
	void EnableBlendingAdditive();
	void EnableBlendingAlphablend();

	VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shaderModule);
	VkPipeline BuildPipeline(VkDevice device);

	std::vector<VkPipelineShaderStageCreateInfo> ShaderStages;

	VkPipelineInputAssemblyStateCreateInfo InputAssembly;
	VkPipelineRasterizationStateCreateInfo Rasterizer;
	VkPipelineColorBlendAttachmentState ColorBlendAttachment;
	VkPipelineMultisampleStateCreateInfo Multisampling;
	VkPipelineLayout PipelineLayout;
	VkPipelineDepthStencilStateCreateInfo DepthStencil;
	VkPipelineRenderingCreateInfo RenderInfo;
	VkFormat ColorAttachmentFormat;

	VkPipeline gradientPipeline;
	VkPipelineLayout gradientPipelineLayout;

	std::vector<ComputeEffect> backgroundEffects;
};

#endif //VRAKTAL_CORE_GPU_PIPELINE_IMPL_VULKAN_H
