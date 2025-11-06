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
        vk::raii::Device& device;
        vk::raii::DescriptorPool pool;

    public:
        explicit Impl(DescriptorPool& p, vk::raii::Device& dev,
            const DescriptorPoolCreateInfo& info);
        ~Impl();

        std::vector<vk::raii::DescriptorSet> AllocateDescriptorSets(
            const std::vector<vk::raii::DescriptorSetLayout*>& layouts, uint32_t count);

        vk::raii::DescriptorPool& GetPool();
        const vk::raii::DescriptorPool& GetPool() const;
    };
}

#endif //VRAKTAL_CORE_GPU_VULKAN_DESCRIPTORPOOL_H