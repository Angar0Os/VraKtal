#include "../src/core/gpu/vulkan/descriptorPool_impl.h"
#include "../src/core/gpu/vulkan/descriptorSetLayout_impl.h"
#include "../src/core/gpu/vulkan/device_impl.h"
#include "../src/core/gpu_detail/converters.h"

core::gpu::DescriptorPool::Impl::Impl(core::gpu::DescriptorPool& _pool,
	const core::gpu::Device* _device)
	: parent(_pool), device(_device), pool(nullptr)
{

	std::vector<vk::DescriptorPoolSize> poolSizes = {
		{ vk::DescriptorType::eUniformBuffer,            1024 },
		{ vk::DescriptorType::eStorageBuffer,            1024 },
		{ vk::DescriptorType::eCombinedImageSampler,     2048 },
		{ vk::DescriptorType::eSampledImage,             1024 },
		{ vk::DescriptorType::eStorageImage,             512  },
		{ vk::DescriptorType::eUniformTexelBuffer,       256  },
		{ vk::DescriptorType::eStorageTexelBuffer,       256  },
		{ vk::DescriptorType::eSampler,                  512  },
		{ vk::DescriptorType::eInputAttachment,          256  },
		{ vk::DescriptorType::eAccelerationStructureKHR, 32   }
	};

	auto createInfo = vk::DescriptorPoolCreateInfo{};

	createInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
	createInfo.maxSets = 2048;
	createInfo.poolSizeCount = uint32_t(poolSizes.size());
	createInfo.pPoolSizes = poolSizes.data();

	pool = vk::raii::DescriptorPool(device->GetImpl().device, createInfo);
}

core::gpu::DescriptorPool::Impl::~Impl() = default;

std::vector<vk::raii::DescriptorSet*> core::gpu::DescriptorPool::Impl::AllocateDescriptorSets(
	const std::vector<vk::raii::DescriptorSetLayout*>& layouts, uint32_t count)
{
	std::vector<vk::DescriptorSetLayout> vkLayouts;
	vkLayouts.reserve(layouts.size());

	for (auto* layout : layouts)
	{
		vkLayouts.push_back(**layout);
	}

	vk::DescriptorSetAllocateInfo allocInfo{};
	allocInfo.descriptorPool = *pool;
	allocInfo.descriptorSetCount = count;
	allocInfo.pSetLayouts = vkLayouts.data();

	auto newSets = device->GetImpl().device.allocateDescriptorSets(allocInfo);

	std::vector<vk::raii::DescriptorSet*> result;
	result.reserve(newSets.size());

	size_t startIndex = allocatedSets.size();
	for (auto& set : newSets)
	{
		allocatedSets.push_back(std::move(set));
	}

	for (size_t i = startIndex; i < allocatedSets.size(); ++i)
	{
		result.push_back(&allocatedSets[i]);
	}

	return result;
}

vk::raii::DescriptorPool& core::gpu::DescriptorPool::Impl::GetPool()
{
	return pool;
}

const vk::raii::DescriptorPool& core::gpu::DescriptorPool::Impl::GetPool() const
{
	return pool;
}

core::gpu::DescriptorPool::DescriptorPool(const core::gpu::Device* device)
{
	m_impl = std::make_unique<Impl>(*this, device);
}

core::gpu::DescriptorPool::~DescriptorPool() = default;

std::vector<void*> core::gpu::DescriptorPool::AllocateDescriptorSets(
	const std::vector<DescriptorSetLayout*>& layouts, uint32_t count)
{
	std::vector<vk::raii::DescriptorSetLayout*> vkLayouts;
	vkLayouts.reserve(layouts.size());

	for (auto* layout : layouts)
	{
		vkLayouts.push_back(&layout->GetImpl().layout);
	}

	auto descriptorSets = m_impl->AllocateDescriptorSets(vkLayouts, count);

	std::vector<void*> handles;
	handles.reserve(descriptorSets.size());

	for (auto* set : descriptorSets)
	{
		handles.push_back(static_cast<void*>(set));
	}

	return handles;
}

core::gpu::DescriptorPool::Impl& core::gpu::DescriptorPool::GetImpl() const
{
	return *m_impl;
}