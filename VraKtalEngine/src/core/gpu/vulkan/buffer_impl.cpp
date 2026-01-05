#include "../src/core/gpu/vulkan/buffer_impl.h"
#include "../src/core/gpu/vulkan/device_impl.h"
#include "../src/core/gpu_detail/converters.h"

#include <cstring>
#include <stdexcept>

core::gpu::Buffer::Impl::Impl(core::gpu::Buffer& p, const core::gpu::Device* device, const SBufferCreateInfo& info)
	: parent(p), buffer(nullptr), device(device), memory(nullptr), bufferSize(info.size), mappedData(nullptr)
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

	buffer = vk::raii::Buffer(device->GetImpl().device, bufferInfo);

	vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();

	vk::MemoryAllocateFlagsInfo allocFlagsInfo{};
	bool needsDeviceAddress = (info.usage & EBufferUsage::ShaderDeviceAddress) != EBufferUsage::None;

	if (needsDeviceAddress)
	{
		allocFlagsInfo.flags = vk::MemoryAllocateFlagBits::eDeviceAddress;
	}

	vk::MemoryAllocateInfo allocInfo{};
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(
		memRequirements.memoryTypeBits,
		core::gpu_detail::ToVulkan(info.memoryProperties)
	);

	if (needsDeviceAddress)
	{
		allocInfo.pNext = &allocFlagsInfo;
	}

	memory = vk::raii::DeviceMemory(device->GetImpl().device, allocInfo);

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
	vk::PhysicalDeviceMemoryProperties memProperties = device->GetImpl().physicalDevice.getMemoryProperties();

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

uint64_t core::gpu::Buffer::Impl::GetDeviceAddress() const
{
	vk::BufferDeviceAddressInfo addressInfo{};
	addressInfo.buffer = *buffer;
	return device->GetImpl().device.getBufferAddress(addressInfo);
}

uint64_t core::gpu::Buffer::GetDeviceAddress() const
{
	return m_impl->GetDeviceAddress();
}

core::gpu::Buffer::Buffer(const core::gpu::Device* device, const SBufferCreateInfo& info)
{
	m_impl = std::make_unique<Impl>(*this, device, info);
}

core::gpu::Buffer::~Buffer() = default;

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

core::gpu::Buffer::Impl& core::gpu::Buffer::GetImpl() const
{
	return *m_impl;
}