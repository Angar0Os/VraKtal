#include "../src/core/gpu/vulkan/commandBuffer_impl.h"

#include <stdexcept>

core::gpu::CommandBuffer::Impl::Impl(core::gpu::CommandBuffer& p, vk::raii::Device& dev,
    vk::raii::Queue& q, vk::raii::CommandPool& pool, const CommandBufferCreateInfo& info)
    : parent(p), device(dev), queue(q), commandPool(pool),
    isSingleTime(info.singleTime), currentIndex(0)
{
    if (info.count == 0)
    {
        throw std::runtime_error("CommandBuffer count cannot be zero");
    }

    vk::CommandBufferLevel level = (info.level == CommandBufferLevel::Primary)
        ? vk::CommandBufferLevel::ePrimary
        : vk::CommandBufferLevel::eSecondary;

    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool = *commandPool;
    allocInfo.level = level;
    allocInfo.commandBufferCount = info.count;

    commandBuffers = vk::raii::CommandBuffers(device, allocInfo);
}

core::gpu::CommandBuffer::Impl::~Impl()
{
}

vk::raii::CommandBuffer& core::gpu::CommandBuffer::Impl::GetCommandBuffer(uint32_t index)
{
    if (index >= commandBuffers.size())
    {
        throw std::out_of_range("CommandBuffer index out of range");
    }
    return commandBuffers[index];
}

const vk::raii::CommandBuffer& core::gpu::CommandBuffer::Impl::GetCommandBuffer(uint32_t index) const
{
    if (index >= commandBuffers.size())
    {
        throw std::out_of_range("CommandBuffer index out of range");
    }
    return commandBuffers[index];
}

uint32_t core::gpu::CommandBuffer::Impl::GetCount() const
{
    return static_cast<uint32_t>(commandBuffers.size());
}

bool core::gpu::CommandBuffer::Impl::IsSingleTime() const
{
    return isSingleTime;
}

void core::gpu::CommandBuffer::Impl::Begin(uint32_t index)
{
    if (index >= commandBuffers.size())
    {
        throw std::out_of_range("CommandBuffer index out of range");
    }

    currentIndex = index;

    vk::CommandBufferBeginInfo beginInfo{};

    if (isSingleTime)
    {
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    }

    commandBuffers[index].begin(beginInfo);
}

void core::gpu::CommandBuffer::Impl::End(uint32_t index)
{
    if (index >= commandBuffers.size())
    {
        throw std::out_of_range("CommandBuffer index out of range");
    }

    commandBuffers[index].end();
}

void core::gpu::CommandBuffer::Impl::Submit()
{
    if (currentIndex >= commandBuffers.size())
    {
        throw std::runtime_error("No command buffer has been begun");
    }

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &*commandBuffers[currentIndex];

    queue.submit(submitInfo, nullptr);
}

void core::gpu::CommandBuffer::Impl::SubmitAndWait()
{
    Submit();
    queue.waitIdle();
}

core::gpu::CommandBuffer::CommandBuffer(void* device, void* queue, const CommandBufferCreateInfo& info)
{
    auto& vkDevice = *static_cast<vk::raii::Device*>(device);
    auto& vkQueue = *static_cast<vk::raii::Queue*>(queue);
    auto& vkCommandPool = *static_cast<vk::raii::CommandPool*>(info.commandPool);

    m_impl = std::make_unique<Impl>(*this, vkDevice, vkQueue, vkCommandPool, info);
}

core::gpu::CommandBuffer::~CommandBuffer() = default;

core::gpu::CommandBuffer::CommandBuffer(CommandBuffer&&) noexcept = default;
core::gpu::CommandBuffer& core::gpu::CommandBuffer::operator=(CommandBuffer&&) noexcept = default;

void* core::gpu::CommandBuffer::GetHandle(uint32_t index) const
{
    return static_cast<void*>(const_cast<vk::CommandBuffer*>(&(*m_impl->GetCommandBuffer(index))));
}

uint32_t core::gpu::CommandBuffer::GetCount() const
{
    return m_impl->GetCount();
}

void core::gpu::CommandBuffer::Begin(uint32_t index)
{
    m_impl->Begin(index);
}

void core::gpu::CommandBuffer::End(uint32_t index)
{
    m_impl->End(index);
}

void core::gpu::CommandBuffer::Submit()
{
    m_impl->Submit();
}

void core::gpu::CommandBuffer::SubmitAndWait()
{
    m_impl->SubmitAndWait();
}

core::gpu::CommandBuffer::Impl& core::gpu::CommandBuffer::GetImpl()
{
    return *m_impl;
}

const core::gpu::CommandBuffer::Impl& core::gpu::CommandBuffer::GetImpl() const
{
    return *m_impl;
}