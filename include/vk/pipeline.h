#ifndef VRAKTAL_VK_PIPELINE_H
#define VRAKTAL_VK_PIPELINE_H
#pragma once

#include <rhi/pipeline.h>

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>
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
	class VulkanDescriptor;
	class VulkanDevice;
	class VulkanCommandBuffer;

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

		VkPipeline _gradientPipeline;
		VkPipelineLayout _gradientPipelineLayout;
		std::vector<ComputeEffect> backgroundEffects;

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

		void InitBackgroundPipelines(VulkanDescriptor* descriptor, VulkanDevice device, VulkanCommandBuffer* commandBuffer);
	};
}

#endif //VRAKTAL_VK_PIPELINE_H
