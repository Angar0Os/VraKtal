#ifndef VRAKTAL_CORE_GPU_IMAGE_H
#define VRAKTAL_CORE_GPU_IMAGE_H
#pragma once

#include <memory>
#include <cstdint>
#include <core/enum.h>

namespace core::gpu
{
	class Buffer;
	class CommandBuffer;
	class Device;

	struct SImageCreateInfo
	{
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t mipLevels = 1;
		uint32_t arrayLayers = 1;
		TextureFormat format = TextureFormat::RGBA8_SRGB;
		ImageTiling tiling = ImageTiling::Optimal;
		ImageUsage usage = ImageUsage::None;
		EMemoryProperty memoryProperties = EMemoryProperty::DeviceLocal;
		SampleCount samples = SampleCount::e1;
	};

	struct SImageViewCreateInfo
	{
		TextureFormat format = TextureFormat::RGBA8_SRGB;
		uint32_t baseMipLevel = 0;
		uint32_t levelCount = 1;
		uint32_t baseArrayLayer = 0;
		uint32_t layerCount = 1;
		bool isDepth = false;
	};

	struct SPredefinedImageCreateInfo;

	class Image
	{
	private:
		struct Impl;
		std::unique_ptr<Impl> m_impl;

	public:
		Image(const core::gpu::Device* device, const SImageCreateInfo& info);
		Image(const core::gpu::Device* device, const SPredefinedImageCreateInfo& info);

		Image(const core::gpu::Device* device, void* image, uint32_t width,
              uint32_t height, TextureFormat format);

		~Image();

		Impl& GetImpl() const;
	};
}

#endif //VRAKTAL_CORE_GPU_IMAGE_H