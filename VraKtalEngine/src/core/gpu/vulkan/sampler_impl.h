#ifndef VRAKTAL_CORE_GPU_VULKAN_SAMPLER_H
#define VRAKTAL_CORE_GPU_VULKAN_SAMPLER_H
#pragma once

#include <core/gpu/sampler.h>
#include <vulkan/vulkan_raii.hpp>

namespace core::gpu
{
	struct Sampler::Impl
	{
		Sampler& parent;
		const core::gpu::Device* device;
		vk::raii::Sampler sampler;

		explicit Impl(Sampler& p, const core::gpu::Device* device, const SamplerCreateInfo& info);
		~Impl();
	};
}

#endif //VRAKTAL_CORE_GPU_VULKAN_SAMPLER_H