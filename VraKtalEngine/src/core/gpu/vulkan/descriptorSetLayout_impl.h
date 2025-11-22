#ifndef VRAKTAL_CORE_GPU_VULKAN_DESCRIPTORSETLAYOUT_H
#define VRAKTAL_CORE_GPU_VULKAN_DESCRIPTORSETLAYOUT_H
#pragma once

#include <core/gpu/descriptorSetLayout.h>
#include <vulkan/vulkan_raii.hpp>

namespace core::gpu
{
	struct DescriptorSetLayout::Impl
	{
	private:
		DescriptorSetLayout& parent;
		vk::raii::Device& device;
		vk::raii::DescriptorSetLayout layout;

	public:
		explicit Impl(DescriptorSetLayout& p, vk::raii::Device& dev,
			const DescriptorSetLayoutCreateInfo& info);
		~Impl();

		vk::raii::DescriptorSetLayout& GetLayout();
		const vk::raii::DescriptorSetLayout& GetLayout() const;
	};
}

#endif //VRAKTAL_CORE_GPU_VULKAN_DESCRIPTORSETLAYOUT_H