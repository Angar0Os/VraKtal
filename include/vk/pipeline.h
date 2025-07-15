#ifndef VRAKTAL_VK_PIPELINE_H
#define VRAKTAL_VK_PIPELINE_H
#pragma once

#include <rhi/pipeline.h>

#include <vulkan/vulkan.h>

#include <vector>

namespace vkutils
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

namespace vk
{
	class VulkanPipeline : rhi::Pipeline
	{
	public:
		std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;

		VkPipelineInputAssemblyStateCreateInfo _inputAssembly;
		VkPipelineRasterizationStateCreateInfo _rasterizer;
		VkPipelineColorBlendAttachmentState _colorBlendAttachment;
		VkPipelineMultisampleStateCreateInfo _multisampling;
		VkPipelineLayout _pipelineLayout;
		VkPipelineDepthStencilStateCreateInfo _depthStencil;
		VkPipelineRenderingCreateInfo _renderInfo;
		VkFormat _colorAttachmentFormat;

		VulkanPipeline() { Clear(); }

		void Clear() override;
		void SetMultisamplingNone() override;
		void DisableBlending() override;
		void DisableDepthtest() override;
		void EnableBlendingAdditive() override;
		void EnableBlendingAlphablend() override;

		void SetShaders(VkShaderModule vertexShader, VkShaderModule fragmentShader);
		void SetInputTopology(VkPrimitiveTopology topology);
		void SetPolygonMode(VkPolygonMode mode);
		void SetCullMode(VkCullModeFlags cullMode, VkFrontFace frontFace);
		void SetColorAttachmentFormat(VkFormat format);
		void SetDepthFormat(VkFormat format);
		void EnableDepthtest(bool depthWriteEnable, VkCompareOp op);

		VkPipeline BuildPipeline(VkDevice device);

		VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shaderModule);
		VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo();
	};
}

#endif //VRAKTAL_VK_PIPELINE_H
