#ifndef VRAKTAL_CORE_GPU_BUFFER_H
#define VRAKTAL_CORE_GPU_BUFFER_H
#pragma once

#include <memory>
#include <core/enum.h>

namespace core::gpu
{
    class Device;

    struct SBufferCreateInfo
    {
        size_t size = 0;
        EBufferUsage usage = EBufferUsage::None;
        EMemoryProperty memoryProperties = EMemoryProperty::None;
    };

    class Buffer
    {
    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;

    public:
        Buffer(const core::gpu::Device* _device, const SBufferCreateInfo& _info);
        ~Buffer();

        size_t GetSize() const;

        uint64_t GetDeviceAddress() const;

        void Map(void** data);
        void Unmap();

        void CopyFrom(const void* _data, size_t _size, size_t _offset = 0);

        Impl& GetImpl() const;
    };
}

#endif //VRAKTAL_CORE_GPU_BUFFER_H