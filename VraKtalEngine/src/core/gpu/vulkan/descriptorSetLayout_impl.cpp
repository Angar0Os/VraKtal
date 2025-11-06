#include "../src/core/gpu/vulkan/descriptorSetLayout_impl.h"
#include "../src/core/gpu_detail/converters.h"

#include <stdexcept>

core::gpu::DescriptorSetLayout::Impl::Impl(core::gpu::DescriptorSetLayout& p,
    vk::raii::Device& dev, const DescriptorSetLayoutCreateInfo& info)
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

    layout = vk::raii::DescriptorSetLayout(device, layoutInfo);
}

core::gpu::DescriptorSetLayout::Impl::~Impl() = default;

vk::raii::DescriptorSetLayout& core::gpu::DescriptorSetLayout::Impl::GetLayout()
{
    return layout;
}

const vk::raii::DescriptorSetLayout& core::gpu::DescriptorSetLayout::Impl::GetLayout() const
{
    return layout;
}

core::gpu::DescriptorSetLayout::DescriptorSetLayout(void* device, const DescriptorSetLayoutCreateInfo& info)
{
    auto& vkDevice = *static_cast<vk::raii::Device*>(device);
    m_impl = std::make_unique<Impl>(*this, vkDevice, info);
}

core::gpu::DescriptorSetLayout::~DescriptorSetLayout() = default;

core::gpu::DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other) noexcept = default;
core::gpu::DescriptorSetLayout& core::gpu::DescriptorSetLayout::operator=(DescriptorSetLayout&& other) noexcept = default;

void* core::gpu::DescriptorSetLayout::GetHandle() const
{
    return static_cast<void*>(&m_impl->GetLayout());
}

core::gpu::DescriptorSetLayout::Impl& core::gpu::DescriptorSetLayout::GetImpl()
{
    return *m_impl;
}

const core::gpu::DescriptorSetLayout::Impl& core::gpu::DescriptorSetLayout::GetImpl() const
{
    return *m_impl;
}