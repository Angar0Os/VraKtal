#ifndef VRAKTAL_CORE_GPU_VULKAN_COMMANDPOOL_H
#define VRAKTAL_CORE_GPU_VULKAN_COMMANDPOOL_H
#pragma once

#include <core/gpu/commandPool.h>
#include <vulkan/vulkan_raii.hpp>

namespace core::gpu
{
    struct CommandPool::Impl
    {
    private:
        CommandPool& parent;
        vk::raii::Device& device;
        vk::raii::CommandPool pool;
        uint32_t queueFamilyIndex;

    public:
        explicit Impl(CommandPool& p, vk::raii::Device& dev,
            const CommandPoolCreateInfo& info);
        ~Impl();

        std::vector<vk::raii::CommandBuffer> AllocateCommandBuffers(uint32_t count, bool secondary);

        void Reset(bool releaseResources);

        vk::raii::CommandPool& GetPool();
        uint32_t GetQueueFamilyIndex() const;
    };
}

#endif //VRAKTAL_CORE_GPU_VULKAN_COMMANDPOOL_H