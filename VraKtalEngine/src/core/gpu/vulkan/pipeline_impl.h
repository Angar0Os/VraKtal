#ifndef VRAKTAL_CORE_GPU_VULKAN_PIPELINE_H
#define VRAKTAL_CORE_GPU_VULKAN_PIPELINE_H
#pragma once

#include <core/gpu/pipeline.h>
#include <vulkan/vulkan_raii.hpp>

namespace core::gpu
{
	struct Pipeline::Impl
	{
		Pipeline& parent;
		const core::gpu::Device* device;

		vk::raii::PipelineLayout pipelineLayout;
		vk::raii::Pipeline pipeline;

		explicit Impl(Pipeline& p, const core::gpu::Device* device, const PipelineCreateInfo& info);
		~Impl();
	};
}

#endif // VRAKTAL_CORE_GPU_VULKAN_PIPELINE_H