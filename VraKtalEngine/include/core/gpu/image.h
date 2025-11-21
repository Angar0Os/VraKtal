#ifndef VRAKTAL_CORE_GPU_IMAGE_H
#define VRAKTAL_CORE_GPU_IMAGE_H
#pragma once

#include <memory>
#include <cstdint>
#include <core/enum.h>

namespace core::gpu
{
	class CommandBuffer;
	class Buffer;

	struct ImageCreateInfo
	{
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t mipLevels = 1;
		uint32_t arrayLayers = 1;
		TextureFormat format = TextureFormat::RGBA8_SRGB;
		ImageTiling tiling = ImageTiling::Optimal;
		ImageUsage usage = ImageUsage::None;
		MemoryProperty memoryProperties = MemoryProperty::DeviceLocal;
		SampleCount samples = SampleCount::e1;
	};

	struct ImageViewCreateInfo
	{
		TextureFormat format = TextureFormat::RGBA8_SRGB;
		uint32_t baseMipLevel = 0;
		uint32_t levelCount = 1;
		uint32_t baseArrayLayer = 0;
		uint32_t layerCount = 1;
		bool isDepth = false;
	};

	class Image
	{
	private:
		struct Impl;
		std::unique_ptr<Impl> m_impl;

	public:
		Image(void* device, void* physicalDevice, const ImageCreateInfo& info);
		~Image();

		void* GetHandle() const;
		void* GetViewHandle() const;

		uint32_t GetWidth() const;
		uint32_t GetHeight() const;
		uint32_t GetMipLevels() const;
		uint32_t GetArrayLayers() const;
		TextureFormat GetFormat() const;

		void CreateView(const ImageViewCreateInfo& info);

		void TransitionLayout(CommandBuffer& commandBuffer, ImageLayout oldLayout,
			ImageLayout newLayout, uint32_t mipLevels = 1);

		void CopyFromBuffer(CommandBuffer& commandBuffer, Buffer& buffer,
			uint32_t width, uint32_t height);

		void GenerateMipmaps(CommandBuffer& commandBuffer, uint32_t width,
			uint32_t height, uint32_t mipLevels);

		Impl& GetImpl();
	};
}

#endif //VRAKTAL_CORE_GPU_IMAGE_H