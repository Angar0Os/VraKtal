#include "../src/core/gpu/vulkan/sampler_impl.h"
#include "../src/core/gpu/vulkan/device_impl.h"
#include "../src/core/gpu_detail/converters.h"

core::gpu::Sampler::Impl::Impl(core::gpu::Sampler& _p, const core::gpu::Device* _device, const SamplerCreateInfo& _info)
	: parent(_p), device(_device), sampler(nullptr)
{
	vk::SamplerCreateInfo samplerInfo{};
	samplerInfo.minFilter = core::gpu_detail::ToVulkan(_info.minFilter);
	samplerInfo.magFilter = core::gpu_detail::ToVulkan(_info.magFilter);
	samplerInfo.mipmapMode = core::gpu_detail::ToVulkan(_info.mipmapMode);
	samplerInfo.addressModeU = core::gpu_detail::ToVulkan(_info.addressModeU);
	samplerInfo.addressModeV = core::gpu_detail::ToVulkan(_info.addressModeV);
	samplerInfo.addressModeW = core::gpu_detail::ToVulkan(_info.addressModeW);
	samplerInfo.mipLodBias = _info.mipLodBias;
	samplerInfo.anisotropyEnable = _info.enableAnisotropy ? vk::True : vk::False;
	samplerInfo.maxAnisotropy = _info.maxAnisotropy;
	samplerInfo.compareEnable = _info.enableCompare ? vk::True : vk::False;
	samplerInfo.compareOp = core::gpu_detail::ToVulkan(_info.compareOp);
	samplerInfo.minLod = _info.minLod;
	samplerInfo.maxLod = _info.maxLod;

	sampler = vk::raii::Sampler(_device->GetImpl().device, samplerInfo);
}

core::gpu::Sampler::Impl::~Impl() = default;


core::gpu::Sampler::Sampler(const core::gpu::Device* device, const SamplerCreateInfo& info)
{
	m_impl = std::make_unique<Impl>(*this, device, info);
}

core::gpu::Sampler::~Sampler() = default;

core::gpu::Sampler::Impl& core::gpu::Sampler::GetImpl() const 
{
	return *m_impl;
}