#ifndef VRAKTAL_CORE_GPU_DESCRIPTORSET_H
#define VRAKTAL_CORE_GPU_DESCRIPTORSET_H
#pragma once

#include <memory>
#include <vector>

#include <core/enum.h>

namespace core::gpu
{
	class DescriptorSet
	{
	private:
		struct Impl;
		std::unique_ptr<Impl> m_pImpl;
	public:
		DescriptorSet(void* device, std::vector<void*>& sets, size_t frame);

		DescriptorSet& BindBuffer(BufferHandle buffer, size_t offset, size_t range);

		DescriptorSet& BindImage(SamplerHandle sampler, const TextureSet* texture,
			const TextureSet& defaultTexture, ImageLayout layout = ImageLayout::ShaderReadOnly);

		void Update();
	};
}

#endif //VRAKTAL_CORE_VULKAN_DESCRIPTORSET_H