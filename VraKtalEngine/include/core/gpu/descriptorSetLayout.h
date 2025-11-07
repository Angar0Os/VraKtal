#ifndef VRAKTAL_CORE_GPU_DESCRIPTORSETLAYOUT_H
#define VRAKTAL_CORE_GPU_DESCRIPTORSETLAYOUT_H
#pragma once

#include <memory>
#include <vector>
#include <core/enum.h>

namespace core::gpu
{
    struct DescriptorSetLayoutBinding
    {
        uint32_t binding;
        DescriptorType descriptorType;
        uint32_t descriptorCount = 1;
        core::ShaderStage stageFlags;
    };

    struct DescriptorSetLayoutCreateInfo
    {
        std::vector<DescriptorSetLayoutBinding> bindings;
    };

    class DescriptorSetLayout
    {
    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;

    public:
        DescriptorSetLayout(void* device, const DescriptorSetLayoutCreateInfo& info);
        ~DescriptorSetLayout();

        DescriptorSetLayout(const DescriptorSetLayout&) = delete;
        DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

        DescriptorSetLayout(DescriptorSetLayout&&) noexcept;
        DescriptorSetLayout& operator=(DescriptorSetLayout&&) noexcept;

        void* GetHandle() const;

        Impl& GetImpl();
        const Impl& GetImpl() const;
    };
}

#endif //VRAKTAL_CORE_GPU_DESCRIPTORSETLAYOUT_H