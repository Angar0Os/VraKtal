#ifndef VRAKTAL_CORE_GPU_SAMPLER_H
#define VRAKTAL_CORE_GPU_SAMPLER_H
#pragma once

#include <memory>
#include <core/enum.h>

namespace core::gpu
{
	class Device;

	struct SamplerCreateInfo
	{
		Filter minFilter = Filter::Linear;
		Filter magFilter = Filter::Linear;
		SamplerMipmapMode mipmapMode = SamplerMipmapMode::Linear;
		SamplerAddressMode addressModeU = SamplerAddressMode::Repeat;
		SamplerAddressMode addressModeV = SamplerAddressMode::Repeat;
		SamplerAddressMode addressModeW = SamplerAddressMode::Repeat;
		float mipLodBias = 0.0f;
		bool enableAnisotropy = false;
		float maxAnisotropy = 1.0f;
		bool enableCompare = false;
		CompareOp compareOp = CompareOp::Always;
		float minLod = 0.0f;
		float maxLod = 1000.0f;
	};

	class Sampler
	{
	private:
		struct Impl;
		std::unique_ptr<Impl> m_impl;

	public:
		Sampler(const core::gpu::Device* device, const SamplerCreateInfo& info);
		~Sampler();

		Impl& GetImpl() const;
	};
}

#endif //VRAKTAL_CORE_GPU_SAMPLER_H