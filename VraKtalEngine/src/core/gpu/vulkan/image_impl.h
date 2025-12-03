#ifndef VRAKTAL_CORE_GPU_VULKAN_IMAGE_H
#define VRAKTAL_CORE_GPU_VULKAN_IMAGE_H
#pragma once

#include <core/gpu/image.h>
#include <vulkan/vulkan_raii.hpp>

namespace core::gpu
{
	struct Image::Impl
	{
		Image& parent;
		const Device* device;

		vk::raii::Image image;
		vk::raii::DeviceMemory memory;
		vk::raii::ImageView view;

		uint32_t width;
		uint32_t height;
		uint32_t mipLevels;
		uint32_t arrayLayers;
		TextureFormat format;
		SampleCount samples;

		uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

	public:
        explicit Impl(Image& p, const core::gpu::Device* device,
			const ImageCreateInfo& info);
		~Impl();

		vk::raii::Image& GetImage();
		const vk::raii::Image& GetImage() const;
		vk::raii::ImageView& GetView();
		const vk::raii::ImageView& GetView() const;

		uint32_t GetWidth() const { return width; }
		uint32_t GetHeight() const { return height; }
		uint32_t GetMipLevels() const { return mipLevels; }
		uint32_t GetArrayLayers() const { return arrayLayers; }
		TextureFormat GetFormat() const { return format; }

		void CreateView(const ImageViewCreateInfo& info);

		void TransitionLayout(CommandBuffer& commandBuffer, ImageLayout oldLayout,
			ImageLayout newLayout, uint32_t mipLevels);

		void CopyFromBuffer(CommandBuffer& commandBuffer, Buffer& buffer,
			uint32_t width, uint32_t height);

		void GenerateMipmaps(CommandBuffer& commandBuffer, uint32_t width,
			uint32_t height, uint32_t mipLevels);
	};
}

#endif //VRAKTAL_CORE_GPU_VULKAN_IMAGE_H