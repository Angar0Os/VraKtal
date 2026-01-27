#include "../src/core/gpu/vulkan/accelerationStructure_impl.h"
#include "../src/core/gpu/vulkan/descriptorSet_impl.h"
#include "../src/core/gpu/vulkan/buffer_impl.h"
#include "../src/core/gpu/vulkan/device_impl.h"
#include "../src/core/gpu/vulkan/image_impl.h"
#include "../src/core/gpu/vulkan/sampler_impl.h"
#include "../src/core/gpu/vulkan/texture_impl.h"
#include "../src/core/gpu_detail/converters.h"

#include <core/gpu/buffer.h>
#include <core/gpu/texture.h>
#include <core/gpu/sampler.h>

#include <core/enum.h>

core::gpu::DescriptorSet::Impl::Impl(core::gpu::DescriptorSet& p, const core::gpu::Device* dev, std::vector<vk::raii::DescriptorSet*>& sets, size_t frame)
	: parent(p), device(dev), descriptorSets(sets), currentFrame(frame)
{
	bufferInfos.reserve(8);
	imageInfos.reserve(8);
	writes.reserve(8);
	bindingInfos.reserve(8);
}

core::gpu::DescriptorSet::Impl::~Impl() = default;

template<>
void core::gpu::DescriptorSet::Bind<core::gpu::Texture>(uint32_t binding, const core::gpu::Texture& texture)
{
	size_t infoIndex = imageInfos.size();
	imageInfos.emplace_back(
		texture.GetImpl().,
		texture.GetImpl().image->GetImpl().view,
		core::gpu_detail::ToVulkan(layout)
	);

	bindingInfos.push_back({
		currentBinding++,
		vk::DescriptorType::eCombinedImageSampler,
		infoIndex
	});
}

core::gpu::DescriptorSet& core::gpu::DescriptorSet::Impl::BindBuffer(const Buffer& buffer, size_t offset, size_t range)
{
	size_t infoIndex = bufferInfos.size();
	bufferInfos.emplace_back(
		buffer.GetImpl().buffer,
		static_cast<vk::DeviceSize>(offset),
		static_cast<vk::DeviceSize>(range)
	);

	bindingInfos.push_back({
		currentBinding++,
		vk::DescriptorType::eUniformBuffer,
		infoIndex
		});

	return parent;
}

core::gpu::DescriptorSet& core::gpu::DescriptorSet::Impl::BindImage(const Sampler& sampler, const Texture* texture,
	const Texture& defaultTexture, ImageLayout layout)
{
	
}

core::gpu::DescriptorSet& core::gpu::DescriptorSet::Impl::BindAccelerationStructure(uint32_t frameIndex, const AccelerationStructure& accelStructure)
{
	vk::DescriptorSet descSet = **descriptorSets[frameIndex];
	vk::AccelerationStructureKHR accelStructHandle = **tlasHandle->GetImpl().accelerationStructure;

	vk::WriteDescriptorSetAccelerationStructureKHR accelInfo{};
	accelInfo.accelerationStructureCount = 1;
	accelInfo.pAccelerationStructures = &accelStructHandle;

	vk::WriteDescriptorSet writeDesc{};
	writeDesc.dstSet = descSet;
	writeDesc.dstBinding = 8;
	writeDesc.dstArrayElement = 0;
	writeDesc.descriptorCount = 1;
	writeDesc.descriptorType = vk::DescriptorType::eAccelerationStructureKHR;
	writeDesc.pNext = &accelInfo;

}

void core::gpu::DescriptorSet::Impl::Update()
{
	if (bindingInfos.empty())
		return;

	writes.clear();
	writes.reserve(bindingInfos.size());

	if (currentFrame >= descriptorSets.size() || descriptorSets[currentFrame] == nullptr)
	{
		throw std::runtime_error("Invalid descriptor set index or null descriptor set pointer");
	}

	vk::raii::DescriptorSet* raiiSet = descriptorSets[currentFrame];
	vk::DescriptorSet dstSet = **raiiSet;

	for (const auto& info : bindingInfos)
	{
		vk::WriteDescriptorSet write{};
		write.dstSet = dstSet;
		write.dstBinding = info.binding;
		write.dstArrayElement = 0;
		write.descriptorCount = 1;
		write.descriptorType = info.type;

		if (info.type == vk::DescriptorType::eUniformBuffer)
		{
			write.pBufferInfo = &bufferInfos[info.infoIndex];
			write.pImageInfo = nullptr;
		}
		else if (info.type == vk::DescriptorType::eCombinedImageSampler)
		{
			write.pImageInfo = &imageInfos[info.infoIndex];
			write.pBufferInfo = nullptr;
		}

		writes.push_back(write);
	}

	device->GetImpl().device.updateDescriptorSets(writes, {});

	currentBinding = 0;
	bufferInfos.clear();
	imageInfos.clear();
	writes.clear();
	bindingInfos.clear();
}

core::gpu::DescriptorSet::DescriptorSet(const core::gpu::Device* device, void* setsVector, size_t frame)
{
	auto& vkSets = *static_cast<std::vector<vk::raii::DescriptorSet*>*>(setsVector);

	m_impl = std::make_unique<Impl>(*this, device, vkSets, frame);
}

core::gpu::DescriptorSet::~DescriptorSet() = default;

core::gpu::DescriptorSet& core::gpu::DescriptorSet::BindBuffer(const Buffer& buffer, size_t offset, size_t range)
{
	m_impl->BindBuffer(buffer, offset, range);
	return *this;
}

core::gpu::DescriptorSet& core::gpu::DescriptorSet::BindImage(const Sampler& sampler, const Texture* texture,
	const Texture& defaultTexture, ImageLayout layout)
{
	m_impl->BindImage(sampler, texture, defaultTexture, layout);
	return *this;
}

void core::gpu::DescriptorSet::Update()
{
	m_impl->Update();
}

core::gpu::DescriptorSet::Impl& core::gpu::DescriptorSet::GetImpl() const
{
	return *m_impl;
}
