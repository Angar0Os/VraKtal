#include "../src/core/gpu/vulkan/commandPool_impl.h"
#include "../src/core/gpu_detail/converters.h"\

#include <stdexcept>

core::gpu::CommandPool::Impl::Impl(core::gpu::CommandPool& p,
    vk::raii::Device& dev, const CommandPoolCreateInfo& info)
    : parent(p), device(dev), pool(nullptr), queueFamilyIndex(info.queueFamilyIndex)
{
    vk::CommandPoolCreateInfo poolInfo{};

    poolInfo.flags = core::gpu_detail::ToVulkan(info.flags);
    poolInfo.queueFamilyIndex = info.queueFamilyIndex;

    pool = vk::raii::CommandPool(device, poolInfo);
}

core::gpu::CommandPool::Impl::~Impl() = default;

std::vector<vk::raii::CommandBuffer> core::gpu::CommandPool::Impl::AllocateCommandBuffers(
    uint32_t count, bool secondary)
{
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool = *pool;
    allocInfo.level = secondary ? vk::CommandBufferLevel::eSecondary : vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = count;

    return device.allocateCommandBuffers(allocInfo);
}

void core::gpu::CommandPool::Impl::Reset(bool releaseResources)
{
    vk::CommandPoolResetFlags flags;
    if (releaseResources)
    {
        flags |= vk::CommandPoolResetFlagBits::eReleaseResources;
    }

    pool.reset(flags);
}

vk::raii::CommandPool& core::gpu::CommandPool::Impl::GetPool()
{
    return pool;
}

uint32_t core::gpu::CommandPool::Impl::GetQueueFamilyIndex() const
{
    return queueFamilyIndex;
}

core::gpu::CommandPool::CommandPool(void* device, const CommandPoolCreateInfo& info)
{
    auto& vkDevice = *static_cast<vk::raii::Device*>(device);
    m_impl = std::make_unique<Impl>(*this, vkDevice, info);
}

core::gpu::CommandPool::~CommandPool() = default;

core::gpu::CommandPool::CommandPool(CommandPool&& other) noexcept = default;
core::gpu::CommandPool& core::gpu::CommandPool::operator=(CommandPool&& other) noexcept = default;

std::vector<void*> core::gpu::CommandPool::AllocateCommandBuffers(uint32_t count, bool secondary)
{
    auto commandBuffers = m_impl->AllocateCommandBuffers(count, secondary);

    std::vector<void*> handles;
    handles.reserve(commandBuffers.size());

    for (auto& cmdBuffer : commandBuffers)
    {
        handles.push_back(static_cast<void*>(&cmdBuffer));
    }

    return handles;
}

void core::gpu::CommandPool::Reset(bool releaseResources)
{
    m_impl->Reset(releaseResources);
}

void* core::gpu::CommandPool::GetHandle() const
{
    return static_cast<void*>(const_cast<vk::raii::CommandPool*>(&m_impl->GetPool()));
}

core::gpu::CommandPool::Impl& core::gpu::CommandPool::GetImpl()
{
    return *m_impl;
}

const core::gpu::CommandPool::Impl& core::gpu::CommandPool::GetImpl() const
{
    return *m_impl;
}