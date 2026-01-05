#ifndef VRAKTAL_CORE_GPU_DESCRIPTORSETLAYOUT_H
#define VRAKTAL_CORE_GPU_DESCRIPTORSETLAYOUT_H
#pragma once

#include <memory>
#include <vector>

#include <core/enum.h>

namespace core::gpu
{
	class Device;

	struct SDescriptorSetLayoutBinding
	{
		uint32_t binding;
		EDescriptorType descriptorType;
		uint32_t descriptorCount = 1;
		core::ShaderStage stageFlags;
	};

	struct SDescriptorSetLayoutCreateInfo
	{
		std::vector<SDescriptorSetLayoutBinding> bindings;
	};

	class DescriptorSetLayout
	{
	private:
		struct Impl;
		std::unique_ptr<Impl> m_impl;

	public:
		DescriptorSetLayout(const core::gpu::Device* device, const SDescriptorSetLayoutCreateInfo& info);
		~DescriptorSetLayout();

		DescriptorSetLayout(const DescriptorSetLayout&) = delete;
		DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

		DescriptorSetLayout(DescriptorSetLayout&&) noexcept;
		DescriptorSetLayout& operator=(DescriptorSetLayout&&) noexcept;

		Impl& GetImpl() const;
	};
}

#endif //VRAKTAL_CORE_GPU_DESCRIPTORSETLAYOUT_H