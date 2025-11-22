#ifndef VRAKTAL_CORE_GPU_VULKAN_SAMPLER_H
#define VRAKTAL_CORE_GPU_VULKAN_SAMPLER_H
#pragma once

#include <core/gpu/sampler.h>
#include <vulkan/vulkan_raii.hpp>

namespace core::gpu
{
	struct Sampler::Impl
	{
	private:
		Sampler& parent;
		vk::raii::Device& device;
		vk::raii::Sampler sampler;

	public:
		explicit Impl(Sampler& p, vk::raii::Device& dev, const SamplerCreateInfo& info);
		~Impl();

		vk::raii::Sampler& GetSampler();
	};
}

#endif //VRAKTAL_CORE_GPU_VULKAN_SAMPLER_H