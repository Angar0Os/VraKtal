#ifndef VRAKTAL_VK_DESCRIPTORS_H
#define VRAKTAL_VK_DESCRIPTORS_H
#pragma once

#include <rhi/descriptors.h>

#include <vector>
#include <vulkan/vulkan.h>

#include <span>
#include <deque>

namespace vk
{
	struct DescriptorAllocator : public rhi::DescriptorAllocator
	{
		struct PoolSizeRatio
		{
			VkDescriptorType type;
			float ratio;
		};

		VkDescriptorPool pool;

		void InitPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);
		void ClearDescriptors(VkDevice device) override;
		void DestroyPool(VkDevice device) override;

		VkDescriptorSet Allocate(VkDevice device, VkDescriptorSetLayout layout) override;
	};

	struct DescriptorLayoutBuilder : public rhi::DescriptorLayoutBuilder
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings;

		void AddBinding(uint32_t binding, VkDescriptorType type) override;
		void Clear() override;

		VkDescriptorSetLayout Build(VkDevice device, VkShaderStageFlags shaderFlags, void* pNext = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0) override;
	};

	struct DescriptorWriter : public rhi::DescriptorWriter
	{
		std::deque<VkDescriptorImageInfo> imageInfos;
		std::deque<VkDescriptorBufferInfo> bufferInfos;
		std::vector<VkWriteDescriptorSet> writes;

		void WriteImage(int binding, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type) override;
		void WriteBuffer(int binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type) override;

		void Clear() override;
		void UpdateSet(VkDevice device, VkDescriptorSet set) override;
	};

	struct DescriptorAllocatorGrowable : public rhi::DescriptorAllocatorGrowable
	{
	public:
		struct PoolSizeRatio
		{
			VkDescriptorType type;
			float ratio;
		};

		void Init(VkDevice device, uint32_t initialSets, std::span<PoolSizeRatio> poolRatios);
		void ClearPools(VkDevice device) override;
		void DestroyPools(VkDevice device) override;

		VkDescriptorSet Allocate(VkDevice device, VkDescriptorSetLayout layout, void* pNext = nullptr) override;

	private:
		VkDescriptorPool GetPool(VkDevice device) override;
		VkDescriptorPool CreatePool(VkDevice device, uint32_t setCount, std::span<PoolSizeRatio> poolRatios);

		std::vector<PoolSizeRatio> ratios;
		std::vector<VkDescriptorPool> fullPools;
		std::vector<VkDescriptorPool> readyPools;
		uint32_t setsPerPool;
	};
}

#endif //VRAKTAL_VK_DESCRIPTORS_H