#ifndef VRAKTAL_CORE_GPU_COMMANDPOOL_H
#define VRAKTAL_CORE_GPU_COMMANDPOOL_H
#pragma once

#include <memory>
#include <vector>

#include <core/enum.h>

namespace core::gpu
{
    class Device;

    struct CommandPoolCreateInfo
    {
        uint32_t queueFamilyIndex;
        CommandPoolCreateFlags flags = CommandPoolCreateFlags::None;
    };

    class CommandPool
    {
    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;

    public:
        CommandPool(const core::gpu::Device* _device, const CommandPoolCreateInfo& _info);
        ~CommandPool();

        CommandPool(const CommandPool&) = delete;
        CommandPool& operator=(const CommandPool&) = delete;

        CommandPool(CommandPool&&) noexcept;
        CommandPool& operator=(CommandPool&&) noexcept;

        std::vector<void*> AllocateCommandBuffers(uint32_t _count, bool _secondary = false);

        void Reset(bool _releaseResources = false);

        Impl& GetImpl() const;
    };
}

#endif //VRAKTAL_CORE_GPU_COMMANDPOOL_H