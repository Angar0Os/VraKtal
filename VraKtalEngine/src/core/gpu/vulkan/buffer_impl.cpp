#include "../src/core/gpu/vulkan/buffer_impl.h"
#include "../src/core/gpu_detail/converters.h"

#include <cstring>
#include <stdexcept>

core::gpu::Buffer::Impl::Impl(core::gpu::Buffer& p, vk::raii::Device& dev,
    vk::raii::PhysicalDevice& physDev, const BufferCreateInfo& info)
    : parent(p), device(dev), physicalDevice(physDev),
    buffer(nullptr), memory(nullptr), bufferSize(info.size), mappedData(nullptr)
{
    if (info.size == 0)
    {
        throw std::runtime_error("Buffer size cannot be zero");
    }

    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.flags = {};
    bufferInfo.size = static_cast<vk::DeviceSize>(info.size);
    bufferInfo.usage = core::gpu_detail::ToVulkan(info.usage);
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;

    buffer = vk::raii::Buffer(device, bufferInfo);

    vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(
        memRequirements.memoryTypeBits,
        core::gpu_detail::ToVulkan(info.memoryProperties)
    );

    memory = vk::raii::DeviceMemory(device, allocInfo);

    buffer.bindMemory(*memory, 0);
}

core::gpu::Buffer::Impl::~Impl()
{
    if (mappedData)
    {
        Unmap();
    }
}

uint32_t core::gpu::Buffer::Impl::FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type");
}

vk::raii::Buffer& core::gpu::Buffer::Impl::GetBuffer()
{
    return buffer;
}

const vk::raii::Buffer& core::gpu::Buffer::Impl::GetBuffer() const
{
    return buffer;
}

size_t core::gpu::Buffer::Impl::GetSize() const
{
    return bufferSize;
}

void core::gpu::Buffer::Impl::Map(void** data)
{
    if (mappedData)
    {
        *data = mappedData;
        return;
    }

    mappedData = memory.mapMemory(0, bufferSize);
    *data = mappedData;
}

void core::gpu::Buffer::Impl::Unmap()
{
    if (mappedData)
    {
        memory.unmapMemory();
        mappedData = nullptr;
    }
}

void core::gpu::Buffer::Impl::CopyFrom(const void* data, size_t size, size_t offset)
{
    if (offset + size > bufferSize)
    {
        throw std::runtime_error("Copy operation exceeds buffer size");
    }

    void* mappedMem = nullptr;
    Map(&mappedMem);

    std::memcpy(static_cast<char*>(mappedMem) + offset, data, size);
}

core::gpu::Buffer::Buffer(void* device, void* physicalDevice, const BufferCreateInfo& info)
{
    auto& vkDevice = *static_cast<vk::raii::Device*>(device);
    auto& vkPhysicalDevice = *static_cast<vk::raii::PhysicalDevice*>(physicalDevice);

    m_impl = std::make_unique<Impl>(*this, vkDevice, vkPhysicalDevice, info);
}

core::gpu::Buffer::~Buffer() = default;

void* core::gpu::Buffer::GetHandle() const
{
    return static_cast<void*>(const_cast<vk::Buffer*>(&(*m_impl->GetBuffer())));
}

size_t core::gpu::Buffer::GetSize() const
{
    return m_impl->GetSize();
}

void core::gpu::Buffer::Map(void** data)
{
    m_impl->Map(data);
}

void core::gpu::Buffer::Unmap()
{
    m_impl->Unmap();
}

void core::gpu::Buffer::CopyFrom(const void* data, size_t size, size_t offset)
{
    m_impl->CopyFrom(data, size, offset);
}

core::gpu::Buffer::Impl& core::gpu::Buffer::GetImpl()
{
    return *m_impl;
}