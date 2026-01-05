#ifndef VRAKTAL_CORE_GPU_PIPELINE_H
#define VRAKTAL_CORE_GPU_PIPELINE_H
#pragma once

#include <memory>
#include <vector>
#include <string>
#include <core/enum.h>

namespace core::gpu
{
	class Device;
	class DescriptorSetLayout;

	struct SVertexInputBinding
	{
		uint32_t binding;
		uint32_t stride;
		VertexInputRate inputRate;
	};

	struct SVertexInputAttribute
	{
		uint32_t location;
		uint32_t binding;
		TextureFormat format;
		uint32_t offset;
	};

	struct ShaderStage
	{
		ShaderStageFlags stage;
		std::vector<char> code;
		std::string entryPoint = "main";
	};

	struct PushConstantRange
	{
		uint32_t stageFlags;
		uint32_t offset;
		uint32_t size;
	};

	struct PipelineCreateInfo
	{
		std::vector<ShaderStage> shaderStages;
		std::vector<SVertexInputBinding> vertexBindings;
		std::vector<SVertexInputAttribute> vertexAttributes;

		PrimitiveTopology topology;

		PolygonMode polygonMode;
		CullMode cullMode;

		FrontFace frontFace;

		CompareOp depthCompareOp;

		bool depthTestEnable;
		bool depthWriteEnable;
		bool blendEnable;

		SampleCount samples;

		TextureFormat depthAttachmentFormat;

		std::vector<TextureFormat> colorAttachmentFormats;
		std::vector<DescriptorSetLayout*> descriptorSetLayouts;
		std::vector<PushConstantRange> pushConstantRanges;
		std::vector<DynamicState> dynamicStates;
	};

	class Pipeline
	{
	private:
		struct Impl;
		std::unique_ptr<Impl> m_impl;

	public:
		Pipeline(const core::gpu::Device* device, const PipelineCreateInfo& info);
		~Pipeline();

		Pipeline(const Pipeline&) = delete;
		Pipeline& operator=(const Pipeline&) = delete;

		Pipeline(Pipeline&&) noexcept;
		Pipeline& operator=(Pipeline&&) noexcept;

		Impl& GetImpl() const;
	};
}

#endif // VRAKTAL_CORE_GPU_PIPELINE_H