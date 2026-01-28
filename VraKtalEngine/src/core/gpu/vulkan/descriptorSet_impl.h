#ifndef VRAKTAL_CORE_GPU_VULKAN_DESCRIPTORSET_H
#define VRAKTAL_CORE_GPU_VULKAN_DESCRIPTORSET_H
#pragma once

#include <core/gpu/descriptorSet.h>
#include <vulkan/vulkan_raii.hpp>

namespace core::gpu
{
	class Buffer;
	class Sampler;
	class Texture;

	struct DescriptorSet::Impl
	{
		DescriptorSet& parent;

		vk::raii::DescriptorSet descriptorSet = nullptr;

		std::vector<vk::DescriptorBufferInfo> bufferInfos;
		std::vector<vk::DescriptorImageInfo> imageInfos;
		std::vector<vk::WriteDescriptorSetAccelerationStructureKHR> asInfos;
		std::vector<vk::WriteDescriptorSet> writes;

		struct BindingInfo
		{
			uint32_t binding;
			vk::DescriptorType type;
			size_t infoIndex;
		};
		std::vector<BindingInfo> bindingInfos;

		explicit Impl(DescriptorSet& p, const core::gpu::Device* device, size_t frame);
		~Impl();
	};
}
#endif //VRAKTAL_CORE_GPU_VULKAN_DESCRIPTORSET_H
