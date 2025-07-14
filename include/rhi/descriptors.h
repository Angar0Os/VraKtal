#ifndef VRAKTAL_RHI_DESCRIPTORS_H
#define VRAKTAL_RHI_DESCRIPTORS_H
#pragma once

#include <vector>
#include <vulkan/vulkan.h>

#include <span>
#include <deque>

namespace rhi
{
	struct PoolSizeRatio;

	struct DescriptorAllocator
	{
		virtual void InitPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios) = 0;
		virtual void ClearDescriptors(VkDevice device) = 0;
		virtual void DestroyPool(VkDevice device) = 0;

		virtual VkDescriptorSet Allocate(VkDevice device, VkDescriptorSetLayout layout) = 0;
	};

	struct DescriptorLayoutBuilder
	{
		virtual void AddBinding(uint32_t binding, VkDescriptorType type) = 0;
		virtual void Clear() = 0;

		virtual VkDescriptorSetLayout Build(VkDevice device, VkShaderStageFlags shaderFlags, void* pNext = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0) = 0;
	};

	struct DescriptorWriter
	{
		virtual void WriteImage(int binding, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type) = 0;
		virtual void WriteBuffer(int binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type) = 0;

		virtual void Clear() = 0;
		virtual void UpdateSet() = 0;
	};

	struct DescriptorAllocatorGrowable
	{
	public:
		virtual void Init() = 0;
		virtual void ClearPools() = 0;
		virtual void DestroyPools() = 0;

		virtual VkDescriptorSet Allocate(VkDevice device, VkDescriptorSetLayout layout) = 0;
		
		virtual VkDescriptorPool GetPool(VkDevice device) = 0;
		virtual VkDescriptorPool CreatePool(VkDevice device, uint32_t setCount, std::span<PoolSizeRatio> poolRatios) = 0;
	};
}

#endif //VRAKTAL_RHI_DESCRIPTORS_H
