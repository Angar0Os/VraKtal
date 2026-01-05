#ifndef VRAKTAL_CORE_GPU_VULKAN_DESCRIPTORSETLAYOUT_H
#define VRAKTAL_CORE_GPU_VULKAN_DESCRIPTORSETLAYOUT_H
#pragma once

#include <core/gpu/descriptorSetLayout.h>
#include <vulkan/vulkan_raii.hpp>

namespace core::gpu
{
	struct DescriptorSetLayout::Impl
	{
		DescriptorSetLayout& parent;
		const core::gpu::Device* device;
		vk::raii::DescriptorSetLayout layout;

		explicit Impl(DescriptorSetLayout& p, const core::gpu::Device* device,
			const SDescriptorSetLayoutCreateInfo& info);
		~Impl();
	};
}

#endif //VRAKTAL_CORE_GPU_VULKAN_DESCRIPTORSETLAYOUT_H