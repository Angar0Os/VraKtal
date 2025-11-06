#include "../src/core/gpu/vulkan/descriptorPool_impl.h"
#include "../src/core/gpu/vulkan/descriptorSetLayout_impl.h"
#include "../src/core/gpu_detail/converters.h"

#include <stdexcept>

core::gpu::DescriptorPool::Impl::Impl(core::gpu::DescriptorPool& p,
    vk::raii::Device& dev, const DescriptorPoolCreateInfo& info)
    : parent(p), device(dev), pool(nullptr)
{
    std::vector<vk::DescriptorPoolSize> vkPoolSizes;
    vkPoolSizes.reserve(info.poolSizes.size());

    for (const auto& poolSize : info.poolSizes)
    {
        vk::DescriptorPoolSize poolSizeInfo{};

        poolSizeInfo.type = core::gpu_detail::ToVulkan(poolSize.type);
        poolSizeInfo.descriptorCount = poolSize.descriptorCount;

        vkPoolSizes.push_back(poolSizeInfo);
    }

    vk::DescriptorPoolCreateFlags flags;
    if (info.allowFreeDescriptorSet)
    {
        flags |= vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    }

    vk::DescriptorPoolCreateInfo poolInfo{};
    poolInfo.flags = flags;
    poolInfo.maxSets = info.maxSets;
    poolInfo.poolSizeCount = static_cast<uint32_t>(vkPoolSizes.size());
    poolInfo.pPoolSizes = vkPoolSizes.data();

    pool = vk::raii::DescriptorPool(device, poolInfo);
}

core::gpu::DescriptorPool::Impl::~Impl() = default;

std::vector<vk::raii::DescriptorSet> core::gpu::DescriptorPool::Impl::AllocateDescriptorSets(
    const std::vector<vk::raii::DescriptorSetLayout*>& layouts, uint32_t count)
{
    std::vector<vk::DescriptorSetLayout> vkLayouts;
    vkLayouts.reserve(layouts.size());

    for (auto* layout : layouts)
    {
        vkLayouts.push_back(**layout);
    }

    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.descriptorPool = *pool;
    allocInfo.descriptorSetCount = count;
    allocInfo.pSetLayouts = vkLayouts.data();

    return device.allocateDescriptorSets(allocInfo);
}

vk::raii::DescriptorPool& core::gpu::DescriptorPool::Impl::GetPool()
{
    return pool;
}

const vk::raii::DescriptorPool& core::gpu::DescriptorPool::Impl::GetPool() const
{
    return pool;
}

core::gpu::DescriptorPool::DescriptorPool(void* device, const DescriptorPoolCreateInfo& info)
{
    auto& vkDevice = *static_cast<vk::raii::Device*>(device);
    m_impl = std::make_unique<Impl>(*this, vkDevice, info);
}

core::gpu::DescriptorPool::~DescriptorPool() = default;

core::gpu::DescriptorPool::DescriptorPool(DescriptorPool&& other) noexcept = default;
core::gpu::DescriptorPool& core::gpu::DescriptorPool::operator=(DescriptorPool&& other) noexcept = default;

std::vector<void*> core::gpu::DescriptorPool::AllocateDescriptorSets(
    const std::vector<void*>& layouts, uint32_t count)
{
    std::vector<vk::raii::DescriptorSetLayout*> vkLayouts;
    vkLayouts.reserve(layouts.size());

    for (void* layout : layouts)
    {
        vkLayouts.push_back(static_cast<vk::raii::DescriptorSetLayout*>(layout));
    }

    auto descriptorSets = m_impl->AllocateDescriptorSets(vkLayouts, count);

    std::vector<void*> handles;
    handles.reserve(descriptorSets.size());

    for (auto& set : descriptorSets)
    {
        handles.push_back(static_cast<void*>(&set));
    }

    return handles;
}

void* core::gpu::DescriptorPool::GetHandle() const
{
    return static_cast<void*>(const_cast<vk::raii::DescriptorPool*>(&m_impl->GetPool()));
}

core::gpu::DescriptorPool::Impl& core::gpu::DescriptorPool::GetImpl()
{
    return *m_impl;
}
