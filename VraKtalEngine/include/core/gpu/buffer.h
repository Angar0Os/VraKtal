#ifndef VRAKTAL_CORE_GPU_BUFFER_H
#define VRAKTAL_CORE_GPU_BUFFER_H
#pragma once

#include <memory>

namespace core::gpu
{
    class Buffer
    {
    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;

    public:
        Buffer();
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