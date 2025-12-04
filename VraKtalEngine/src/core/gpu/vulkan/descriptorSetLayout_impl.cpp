#include "../src/core/gpu/vulkan/descriptorSetLayout_impl.h"
#include "../src/core/gpu/vulkan/device_impl.h"
#include "../src/core/gpu_detail/converters.h"

#include <stdexcept>

core::gpu::DescriptorSetLayout::Impl::Impl(core::gpu::DescriptorSetLayout& p,
	const core::gpu::Device* dev, const SDescriptorSetLayoutCreateInfo& info)
	: parent(p), device(dev), layout(nullptr)
{
	std::vector<vk::DescriptorSetLayoutBinding> vkBindings;
	vkBindings.reserve(info.bindings.size());

	for (const auto& binding : info.bindings)
	{
		vk::DescriptorSetLayoutBinding bindings{};
		bindings.binding = binding.binding;
		bindings.descriptorType = core::gpu_detail::ToVulkan(binding.descriptorType);
		bindings.descriptorCount = binding.descriptorCount;
		bindings.stageFlags = core::gpu_detail::ToVulkan(binding.stageFlags);
		bindings.pImmutableSamplers = nullptr;

		vkBindings.push_back(bindings);
	}

	vk::DescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.bindingCount = static_cast<uint32_t>(vkBindings.size());
	layoutInfo.pBindings = vkBindings.data();

	layout = vk::raii::DescriptorSetLayout(device->GetImpl().device, layoutInfo);
}

core::gpu::DescriptorSetLayout::Impl::~Impl() = default;

core::gpu::DescriptorSetLayout::DescriptorSetLayout(const core::gpu::Device* device, const SDescriptorSetLayoutCreateInfo& info)
{
	m_impl = std::make_unique<Impl>(*this, device, info);
}

core::gpu::DescriptorSetLayout::~DescriptorSetLayout() = default;

core::gpu::DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other) noexcept = default;
core::gpu::DescriptorSetLayout& core::gpu::DescriptorSetLayout::operator=(DescriptorSetLayout&& other) noexcept = default;

core::gpu::DescriptorSetLayout::Impl& core::gpu::DescriptorSetLayout::GetImpl() const
{
	return *m_impl;
}