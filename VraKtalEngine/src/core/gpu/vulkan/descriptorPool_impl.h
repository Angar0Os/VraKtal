#ifndef VRAKTAL_CORE_GPU_VULKAN_DESCRIPTORPOOL_H
#define VRAKTAL_CORE_GPU_VULKAN_DESCRIPTORPOOL_H
#pragma once

#include <core/gpu/descriptorPool.h>
#include <vulkan/vulkan_raii.hpp>

namespace core::gpu
{
    struct DescriptorPool::Impl
    {
    private:
        DescriptorPool& parent;
        const core::gpu::Device* device;
        vk::raii::DescriptorPool pool;

        std::vector<vk::raii::DescriptorSet> allocatedSets;
    public:
        explicit Impl(DescriptorPool& _pool, const core::gpu::Device* _device,
                      const SDescriptorPoolCreateInfo& _info);
        ~Impl();

        std::vector<vk::raii::DescriptorSet*> AllocateDescriptorSets(
            const std::vector<vk::raii::DescriptorSetLayout*>& _layouts, uint32_t _count);

        vk::raii::DescriptorPool& GetPool();
        const vk::raii::DescriptorPool& GetPool() const;
    };
}

#endif //VRAKTAL_CORE_GPU_VULKAN_DESCRIPTORPOOL_H