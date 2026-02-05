#include "../src/core/gpu/vulkan/accelerationStructure_impl.h"
#include "../src/core/gpu/vulkan/buffer_impl.h"
#include "../src/core/gpu/vulkan/descriptorSet_impl.h"
#include "../src/core/gpu/vulkan/descriptorPool_impl.h"]
#include "../src/core/gpu/vulkan/descriptorSetLayout_impl.h"
#include "../src/core/gpu/vulkan/device_impl.h"
#include "../src/core/gpu/vulkan/image_impl.h"
#include "../src/core/gpu/vulkan/texture_impl.h"

#include <core/gpu/texture.h>

core::gpu::DescriptorSet::Impl::Impl(core::gpu::DescriptorSet& p, const core::gpu::Device* dev, const core::gpu::DescriptorSetLayout* dsLayout)
	: parent(p)
{
	vk::DescriptorSetAllocateInfo allocInfo{};
	allocInfo.descriptorPool = *dev->GetImpl().descriptorPool->GetImpl().pool;
	allocInfo.descriptorSetCount = 1;
	vk::DescriptorSetLayout vkLayout = dsLayout->GetImpl().layout;
	allocInfo.pSetLayouts = &vkLayout;

	auto sets = vk::raii::DescriptorSets(dev->GetImpl().device, allocInfo);
	descriptorSet = std::move(sets[0]);

	bufferInfos.reserve(8);
	imageInfos.reserve(8);
	asInfos.reserve(8);
	writes.reserve(8);
	bindingInfos.reserve(8);
}

core::gpu::DescriptorSet::Impl::~Impl() = default;

template<>
void core::gpu::DescriptorSet::Bind<core::gpu::Texture>(uint32_t binding, const core::gpu::Texture& texture)
{
	size_t infoIndex = m_impl->imageInfos.size();
	m_impl->imageInfos.emplace_back(
		*texture.GetImpl().sampler,
		*texture.GetImpl().image->GetImpl().view,
		texture.GetImpl().image->GetImpl().currentLayout
	);

	m_impl->bindingInfos.push_back({
		binding,
		vk::DescriptorType::eCombinedImageSampler,
		infoIndex
		});
}

template<>
void core::gpu::DescriptorSet::Bind<core::gpu::Buffer>(uint32_t binding, const core::gpu::Buffer& buffer)
{
	size_t infoIndex = m_impl->bufferInfos.size();
	m_impl->bufferInfos.emplace_back(
		buffer.GetImpl().buffer,
		0,
		buffer.GetImpl().bufferSize
	);

	// Peut être changer le type de buffer qu'on ne push pas que des UB
	m_impl->bindingInfos.push_back({
		binding,
		vk::DescriptorType::eUniformBuffer,
		infoIndex
		});
}

template<>
void core::gpu::DescriptorSet::Bind<core::gpu::AccelerationStructure>(uint32_t binding, const core::gpu::AccelerationStructure& accelStructure)
{
	size_t infoIndex = m_impl->asInfos.size();
	m_impl->asInfos.emplace_back();

	m_impl->bindingInfos.push_back({
		binding,
		vk::DescriptorType::eAccelerationStructureKHR,
		infoIndex
		});
}

void core::gpu::DescriptorSet::Update(const core::gpu::Device& device)
{
	if (m_impl->bindingInfos.empty())
		return;

	std::vector<vk::WriteDescriptorSet> writes;
	writes.reserve(m_impl->bindingInfos.size());

	for (const auto& bindingInfo : m_impl->bindingInfos)
	{
		vk::WriteDescriptorSet write{};
		write.dstSet = m_impl->descriptorSet;
		write.dstBinding = bindingInfo.binding;
		write.dstArrayElement = 0;
		write.descriptorCount = 1;
		write.descriptorType = bindingInfo.type;

		switch (bindingInfo.type)
		{
		case vk::DescriptorType::eCombinedImageSampler:
			write.pImageInfo = &m_impl->imageInfos[bindingInfo.infoIndex];
			break;

		case vk::DescriptorType::eUniformBuffer:
			write.pBufferInfo = &m_impl->bufferInfos[bindingInfo.infoIndex];
			break;

		case vk::DescriptorType::eAccelerationStructureKHR:
		{
			vk::WriteDescriptorSetAccelerationStructureKHR asWrite{};
			asWrite.accelerationStructureCount = m_impl->asInfos[bindingInfo.infoIndex].accelerationStructureCount;
			asWrite.pAccelerationStructures = m_impl->asInfos[bindingInfo.infoIndex].pAccelerationStructures;
			write.pNext = &asWrite;
			break;
		}
		}

		writes.push_back(write);
	}

	device.GetImpl().device.updateDescriptorSets(writes, nullptr);

	m_impl->imageInfos.clear();
	m_impl->bufferInfos.clear();
	m_impl->asInfos.clear();
	m_impl->bindingInfos.clear();
}

core::gpu::DescriptorSet::DescriptorSet(const core::gpu::Device* device, const core::gpu::DescriptorSetLayout* dsLayout)
{
	m_impl = std::make_unique<Impl>(*this, device, dsLayout);
}

core::gpu::DescriptorSet::~DescriptorSet() = default;

core::gpu::DescriptorSet::Impl& core::gpu::DescriptorSet::GetImpl() const
{
	return *m_impl;
}
