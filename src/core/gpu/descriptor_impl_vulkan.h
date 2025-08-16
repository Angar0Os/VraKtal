#ifndef VRAKTAL_CORE_GPU_DESCRIPTOR_IMPL_VULKAN_H
#define VRAKTAL_CORE_GPU_DESCRIPTOR_IMPL_VULKAN_H
#pragma once

#include <core/gpu/descriptor.h>

#include <vulkan/vulkan.h>
#include <deque>
#include <span>
#include <vector>

namespace core { namespace rhi { class RenderContext; } }

struct DescriptorAllocator
{
	struct PoolSizeRatio
	{
		VkDescriptorType type;
		float ratio;
	};

	VkDescriptorPool pool = VK_NULL_HANDLE;

	void InitPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);
	void ClearDescriptors(VkDevice device);
	void DestroyPool(VkDevice device);

	VkDescriptorSet Allocate(VkDevice device, VkDescriptorSetLayout layout);
};

struct DescriptorLayoutBuilder
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	void AddBinding(uint32_t binding, VkDescriptorType type);
	void Clear();

	VkDescriptorSetLayout Build(VkDevice device, VkShaderStageFlags shaderFlags, void* pNext = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0);
};

struct DescriptorWriter
{
	std::deque<VkDescriptorImageInfo> imageInfos;
	std::deque<VkDescriptorBufferInfo> bufferInfos;
	std::vector<VkWriteDescriptorSet> writes;

	void WriteImage(int binding, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type);
	void WriteBuffer(int binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type);

	void Clear();
	void UpdateSet(VkDevice device, VkDescriptorSet set);
};

struct DescriptorAllocatorGrowable
{
public:
	struct PoolSizeRatio
	{
		VkDescriptorType type;
		float ratio;
	};

	void Init(VkDevice device, uint32_t initialSets, std::span<PoolSizeRatio> poolRatios);
	void ClearPools(VkDevice device);
	void DestroyPools(VkDevice device);

	VkDescriptorSet Allocate(VkDevice device, VkDescriptorSetLayout layout, void* pNext = nullptr);

private:
	VkDescriptorPool GetPool(VkDevice device);
	VkDescriptorPool CreatePool(VkDevice device, uint32_t setCount, std::span<PoolSizeRatio> poolRatios);

	std::vector<PoolSizeRatio> ratios;
	std::vector<VkDescriptorPool> fullPools;
	std::vector<VkDescriptorPool> readyPools;
	uint32_t setsPerPool;
};

struct core::rhi::gpu::Descriptor::Internal
{
	DescriptorAllocator globalDescriptorAllocator;

	VkDescriptorSet drawImageDescriptors;
	VkDescriptorSetLayout drawImageDescriptorLayout;

	VkDescriptorSetLayout gpuSceneDataDescriptorLayout;

	void InitDescriptor(core::rhi::RenderContext& rCtx);
};

#endif //VRAKTAL_CORE_GPU_DESCRIPTOR_IMPL_VULKAN_H
