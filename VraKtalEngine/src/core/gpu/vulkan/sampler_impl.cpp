#include "../src/core/gpu/vulkan/sampler_impl.h"
#include "../src/core/gpu_detail/converters.h"

core::gpu::Sampler::Impl::Impl(core::gpu::Sampler& p, vk::raii::Device& dev, const SamplerCreateInfo& info)
	: parent(p), device(dev), sampler(nullptr)
{
	vk::SamplerCreateInfo samplerInfo{};
	samplerInfo.minFilter = core::gpu_detail::ToVulkan(info.minFilter);
	samplerInfo.magFilter = core::gpu_detail::ToVulkan(info.magFilter);
	samplerInfo.mipmapMode = core::gpu_detail::ToVulkan(info.mipmapMode);
	samplerInfo.addressModeU = core::gpu_detail::ToVulkan(info.addressModeU);
	samplerInfo.addressModeV = core::gpu_detail::ToVulkan(info.addressModeV);
	samplerInfo.addressModeW = core::gpu_detail::ToVulkan(info.addressModeW);
	samplerInfo.mipLodBias = info.mipLodBias;
	samplerInfo.anisotropyEnable = info.enableAnisotropy ? vk::True : vk::False;
	samplerInfo.maxAnisotropy = info.maxAnisotropy;
	samplerInfo.compareEnable = info.enableCompare ? vk::True : vk::False;
	samplerInfo.compareOp = core::gpu_detail::ToVulkan(info.compareOp);
	samplerInfo.minLod = info.minLod;
	samplerInfo.maxLod = info.maxLod;

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
	return reinterpret_cast<void*>(static_cast<VkSampler>(*m_impl->GetSampler()));
}

core::gpu::Sampler::Impl& core::gpu::Sampler::GetImpl()
{
	return *m_impl;
}