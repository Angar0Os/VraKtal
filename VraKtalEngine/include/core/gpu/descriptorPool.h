#ifndef VRAKTAL_CORE_GPU_DESCRIPTORPOOL_H
#define VRAKTAL_CORE_GPU_DESCRIPTORPOOL_H
#pragma once

#include <memory>
#include <vector>
#include <core/enum.h>
#include <core/gpu/descriptorSetLayout.h>

namespace core::gpu
{
	struct DescriptorPoolSize
	{
		DescriptorType type;
		uint32_t descriptorCount;
	};

	struct DescriptorPoolCreateInfo
	{
		uint32_t maxSets;
		std::vector<DescriptorPoolSize> poolSizes;
		bool allowFreeDescriptorSet = false;
	};

    class DescriptorPool
    {
    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;

    public:
        DescriptorPool(void* device, const DescriptorPoolCreateInfo& info);
        ~DescriptorPool();

        DescriptorPool(const DescriptorPool&) = delete;
        DescriptorPool& operator=(const DescriptorPool&) = delete;

        DescriptorPool(DescriptorPool&&) noexcept;
        DescriptorPool& operator=(DescriptorPool&&) noexcept;

        std::vector<void*> AllocateDescriptorSets(const std::vector<DescriptorSetLayout*>& layouts, uint32_t count);

        void* GetHandle() const;

        Impl& GetImpl();
    };
}

#endif //VRAKTAL_CORE_GPU_DESCRIPTORPOOL_H