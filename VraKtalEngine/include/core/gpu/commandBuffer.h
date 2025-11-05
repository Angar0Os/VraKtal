#ifndef VRAKTAL_CORE_GPU_COMMAND_BUFFER_H
#define VRAKTAL_CORE_GPU_COMMAND_BUFFER_H
#pragma once

#include <memory>
#include <cstddef>
#include <core/enum.h>

namespace core::gpu
{
    struct CommandBufferCreateInfo
    {
        void* commandPool = nullptr;
        CommandBufferLevel level = CommandBufferLevel::Primary;
        uint32_t count = 1;
        bool singleTime = false;
    };

    class CommandBuffer
    {
    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;

    public:
        CommandBuffer(void* device, void* queue, const CommandBufferCreateInfo& info);
        ~CommandBuffer();

        CommandBuffer(const CommandBuffer&) = delete;
        CommandBuffer& operator=(const CommandBuffer&) = delete;

        CommandBuffer(CommandBuffer&&) noexcept;
        CommandBuffer& operator=(CommandBuffer&&) noexcept;

        void* GetHandle(uint32_t index = 0) const;
        uint32_t GetCount() const;

        void Begin(uint32_t index = 0);
        void End(uint32_t index = 0);

        void Submit();
        void SubmitAndWait();

        Impl& GetImpl();
        const Impl& GetImpl() const;
    };
}

#endif //VRAKTAL_CORE_GPU_COMMAND_BUFFER_H