#ifndef VRAKTAL_CORE_GPU_VULKAN_BUFFER_H
#define VRAKTAL_CORE_GPU_VULKAN_BUFFER_H
#pragma once

#include <core/gpu/buffer.h>
#include <vulkan/vulkan_raii.hpp>

namespace core::gpu
{
	class Device;

	struct Buffer::Impl
	{
		Buffer& parent;
        const core::gpu::Device* device;

		vk::raii::Buffer buffer;
		vk::raii::DeviceMemory memory;


		size_t bufferSize;
		void* mappedData;

		uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

		explicit Impl(Buffer& p, const core::gpu::Device* device,
			const SBufferCreateInfo& info);

		~Impl();

		size_t GetSize() const;

		uint64_t GetDeviceAddress() const;

		void Map(void** data);
		void Unmap();
		void CopyFrom(const void* data, size_t size, size_t offset);
	};
}

#endif //VRAKTAL_CORE_GPU_VULKAN_BUFFER_H