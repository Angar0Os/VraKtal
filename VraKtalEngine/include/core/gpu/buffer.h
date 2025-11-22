#ifndef VRAKTAL_CORE_GPU_BUFFER_H
#define VRAKTAL_CORE_GPU_BUFFER_H
#pragma once

#include <memory>
#include <core/enum.h>

namespace core::gpu
{
	struct BufferCreateInfo
	{
		size_t size = 0;
		BufferUsage usage = BufferUsage::None;
		MemoryProperty memoryProperties = MemoryProperty::None;
	};

	class Buffer
	{
	private:
		struct Impl;
		std::unique_ptr<Impl> m_impl;

	public:
		Buffer(void* device, void* physicalDevice, const BufferCreateInfo& info);
		~Buffer();

		void* GetHandle() const;
		size_t GetSize() const;

		void Map(void** data);
		void Unmap();

		void CopyFrom(const void* data, size_t size, size_t offset = 0);

		Impl& GetImpl();
	};
}

#endif //VRAKTAL_CORE_GPU_BUFFER_H