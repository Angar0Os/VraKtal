#ifndef VRAKTAL_CORE_GPU_DESCRIPTORPOOL_H
#define VRAKTAL_CORE_GPU_DESCRIPTORPOOL_H
#pragma once

#include <memory>
#include <vector>

#include <core/enum.h>

namespace core::gpu
{
    class Device;
    class DescriptorSetLayout;

    struct SDescriptorPoolSize
    {
        EDescriptorType type;
        uint32_t descriptorCount;
    };

    struct SDescriptorPoolCreateInfo
    {
        uint32_t maxSets;
        std::vector<SDescriptorPoolSize> poolSizes;
        bool allowFreeDescriptorSet = false;
    };

    class DescriptorPool
    {
    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;

    public:
        DescriptorPool(const core::gpu::Device* _device, const SDescriptorPoolCreateInfo& _info);
        ~DescriptorPool();

        DescriptorPool(const DescriptorPool&) = delete;
        DescriptorPool& operator=(const DescriptorPool&) = delete;

        DescriptorPool(DescriptorPool&&) noexcept;
        DescriptorPool& operator=(DescriptorPool&&) noexcept;

        std::vector<void*> AllocateDescriptorSets(const std::vector<DescriptorSetLayout*>& _layouts, uint32_t _count);

        Impl& GetImpl() const;
    };
}

#endif //VRAKTAL_CORE_GPU_DESCRIPTORPOOL_H