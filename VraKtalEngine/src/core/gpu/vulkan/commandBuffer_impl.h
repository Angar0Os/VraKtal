#ifndef VRAKTAL_CORE_GPU_VULKAN_COMMAND_BUFFER_H
#define VRAKTAL_CORE_GPU_VULKAN_COMMAND_BUFFER_H
#pragma once

#include <core/gpu/commandBuffer.h>

#include <vulkan/vulkan_raii.hpp>
#include <vector>

namespace core::gpu
{
    struct CommandBuffer::Impl
    {
    private:
        CommandBuffer& parent;
        vk::raii::Device& device;
        vk::raii::Queue& queue;
        vk::raii::CommandPool& commandPool;

        std::vector<vk::raii::CommandBuffer> commandBuffers;
        bool isSingleTime;
        uint32_t currentIndex;

    public:
        explicit Impl(CommandBuffer& p, vk::raii::Device& dev, vk::raii::Queue& q,
            vk::raii::CommandPool& pool, const CommandBufferCreateInfo& info);
        ~Impl();

        vk::raii::CommandBuffer& GetCommandBuffer(uint32_t index = 0);
        const vk::raii::CommandBuffer& GetCommandBuffer(uint32_t index = 0) const;

        uint32_t GetCount() const;
        bool IsSingleTime() const;

        void Begin(uint32_t index);
        void End(uint32_t index);
        void Submit();
        void SubmitAndWait();
    };
}

#endif //VRAKTAL_CORE_GPU_VULKAN_COMMAND_BUFFER_H