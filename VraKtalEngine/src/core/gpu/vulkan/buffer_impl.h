#ifndef VRAKTAL_CORE_GPU_VULKAN_BUFFER_H
#define VRAKTAL_CORE_GPU_VULKAN_BUFFER_H
#pragma once

#include <core/gpu/buffer.h>
#include <vulkan/vulkan_raii.hpp>

namespace core::gpu
{
	struct Buffer::Impl
	{
	private:
		Buffer& parent;
		vk::raii::Device& device;
		vk::raii::PhysicalDevice& physicalDevice;

		vk::raii::Buffer buffer;
		vk::raii::DeviceMemory memory;

		size_t bufferSize;
		void* mappedData;

		uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

	public:
		explicit Impl(Buffer& p, vk::raii::Device& dev, vk::raii::PhysicalDevice& physDev,
			const BufferCreateInfo& info);

		~Impl();

		vk::raii::Buffer& GetBuffer();
		const vk::raii::Buffer& GetBuffer() const;

		size_t GetSize() const;

		uint64_t GetDeviceAddress() const;

		void Map(void** data);
		void Unmap();
		void CopyFrom(const void* data, size_t size, size_t offset);
	};
}

#endif //VRAKTAL_CORE_GPU_VULKAN_BUFFER_H