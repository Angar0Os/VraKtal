#include "../src/core/gpu/vulkan/descriptorSet_impl.h"
#include "../src/core/gpu_detail/converters.h"

#include <core/gpu/buffer.h>
#include <core/gpu/texture.h>
#include <core/gpu/sampler.h>

#include <core/enum.h>

core::gpu::DescriptorSet::Impl::Impl(core::gpu::DescriptorSet& p, vk::raii::Device& dev, std::vector<vk::raii::DescriptorSet>& sets, size_t frame)
	: parent(p), device(dev), descriptorSets(sets), currentFrame(frame)
{
    bufferInfos.reserve(8);
    imageInfos.reserve(8);
    writes.reserve(8);
}

core::gpu::DescriptorSet::Impl::~Impl() = default;

core::gpu::DescriptorSet& core::gpu::DescriptorSet::Impl::BindBuffer(const Buffer& buffer, size_t offset, size_t range)
{
    vk::Buffer vkBuffer = *static_cast<vk::Buffer*>(buffer.GetHandle());

    bufferInfos.emplace_back(
        vkBuffer,
        static_cast<vk::DeviceSize>(offset),
        static_cast<vk::DeviceSize>(range)
    );

    writes.emplace_back(
        descriptorSets[currentFrame],
        currentBinding++,
        0,
        1,
        vk::DescriptorType::eUniformBuffer,
        nullptr,
        &bufferInfos.back(),
        nullptr
    );

    return parent;
}

core::gpu::DescriptorSet& core::gpu::DescriptorSet::Impl::BindImage(const Sampler& sampler,const Texture *texture,
    const Texture& defaultTexture, ImageLayout layout)
{
    auto* vkSampler = static_cast<vk::raii::Sampler*>(sampler.GetHandle());

    const Texture* selectedTexture = (texture && texture->isValid())
        ? texture
        : &defaultTexture;

    auto* vkImageView = static_cast<vk::raii::ImageView*>(selectedTexture->GetImageView());

    imageInfos.emplace_back(
        **vkSampler,
        **vkImageView,
        core::gpu_detail::ToVulkan(layout)
    );

    writes.emplace_back(
        descriptorSets[currentFrame],
        currentBinding++,
        0,
        1,
        vk::DescriptorType::eCombinedImageSampler,
        &imageInfos.back(),
        nullptr,
        nullptr
    );

    return parent;
}

void core::gpu::DescriptorSet::Impl::Update()
{
    if (!writes.empty())
    {
        device.updateDescriptorSets(writes, {});
    }

    currentBinding = 0;
    bufferInfos.clear();
    imageInfos.clear();
    writes.clear();
}

core::gpu::DescriptorSet::DescriptorSet(void* device, std::vector<void*>& sets, size_t frame)
{
    auto& vkDevice = *static_cast<vk::raii::Device*>(device);
    auto& vkSets = *reinterpret_cast<std::vector<vk::raii::DescriptorSet>*>(&sets);

    m_impl = std::make_unique<Impl>(*this, vkDevice, vkSets, frame);
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

core::gpu::DescriptorSet::Impl& core::gpu::DescriptorSet::GetImpl()
{
    return *m_impl;
}
