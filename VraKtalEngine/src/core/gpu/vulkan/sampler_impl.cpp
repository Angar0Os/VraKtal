#include "../src/core/gpu/vulkan/sampler_impl.h"
#include "../src/core/gpu_detail/converters.h"

#include <core/enum.h>

core::gpu::Sampler::Impl::Impl(core::gpu::Sampler& p, vk::raii::Device& dev, const SamplerCreateInfo& info)
    : parent(p), device(dev), sampler(nullptr)
{
    vk::SamplerCreateInfo samplerInfo{
        .magFilter = core::gpu_detail::ToVulkan(info.magFilter),
        .minFilter = core::gpu_detail::ToVulkan(info.minFilter),
        .mipmapMode = core::gpu_detail::ToVulkan(info.mipmapMode),
        .addressModeU = core::gpu_detail::ToVulkan(info.addressModeU),
        .addressModeV = core::gpu_detail::ToVulkan(info.addressModeV),
        .addressModeW = core::gpu_detail::ToVulkan(info.addressModeW),
        .mipLodBias = info.mipLodBias,
        .anisotropyEnable = info.enableAnisotropy ? vk::True : vk::False,
        .maxAnisotropy = info.maxAnisotropy,
        .compareEnable = info.enableCompare ? vk::True : vk::False,
        .compareOp = core::gpu_detail::ToVulkan(info.compareOp),
        .minLod = info.minLod,
        .maxLod = info.maxLod
    };

    sampler = vk::raii::Sampler(device, samplerInfo);
}

core::gpu::Sampler::Impl::~Impl() = default;

vk::raii::Sampler& core::gpu::Sampler::Impl::GetSampler()
{
    return sampler;
}

core::gpu::Sampler::Sampler(void* device, const SamplerCreateInfo& info)
{
    auto& vkDevice = *static_cast<vk::raii::Device*>(device);
    m_impl = std::make_unique<Impl>(*this, vkDevice, info);
}

core::gpu::Sampler::~Sampler() = default;

void* core::gpu::Sampler::GetHandle() const
{
    return static_cast<void*>(&m_impl->GetSampler());
}

core::gpu::Sampler::Impl& core::gpu::Sampler::GetImpl()
{
    return *m_impl;
}