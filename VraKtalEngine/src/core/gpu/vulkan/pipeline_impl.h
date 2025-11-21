#ifndef VRAKTAL_CORE_GPU_VULKAN_PIPELINE_IMPL_H
#define VRAKTAL_CORE_GPU_VULKAN_PIPELINE_IMPL_H
#pragma once

#include <core/gpu/pipeline.h>
#include <vulkan/vulkan_raii.hpp>

namespace core::gpu
{
	struct Pipeline::Impl
	{
	private:
		Pipeline& parent;
		vk::raii::Device& device;

		vk::raii::PipelineLayout pipelineLayout;
		vk::raii::Pipeline pipeline;

	public:
		explicit Impl(Pipeline& p, vk::raii::Device& dev, const PipelineCreateInfo& info);
		~Impl();

		vk::raii::Pipeline& GetPipeline();
		const vk::raii::Pipeline& GetPipeline() const;

		vk::raii::PipelineLayout& GetPipelineLayout();
		const vk::raii::PipelineLayout& GetPipelineLayout() const;
	};
}

#endif // VRAKTAL_CORE_GPU_VULKAN_PIPELINE_IMPL_H