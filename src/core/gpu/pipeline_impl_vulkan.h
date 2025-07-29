#ifndef VRAKTAL_CORE_GPU_PIPELINE_IMPL_VULKAN_H
#define VRAKTAL_CORE_GPU_PIPELINE_IMPL_VULKAN_H
#pragma once

#include <core/gpu/pipeline.h>

#include <vulkan/vulkan.h>
#include <vector>

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
	void Clear();
	void InitBackgroundPipelines() { Clear(); };
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
};

#endif //VRAKTAL_CORE_GPU_PIPELINE_IMPL_VULKAN_H
