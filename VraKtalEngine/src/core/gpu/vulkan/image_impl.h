#ifndef VRAKTAL_CORE_GPU_VULKAN_IMAGE_H
#define VRAKTAL_CORE_GPU_VULKAN_IMAGE_H
#pragma once

#include <variant>

#include <core/gpu/image.h>
#include <vulkan/vulkan_raii.hpp>

namespace core::gpu
{
	struct Image::Impl
	{
		Image& parent;
		const Device* device;

		std::variant< vk::raii::Image, vk::Image> image;
		vk::raii::DeviceMemory memory;
		vk::raii::ImageView view;

		uint32_t width;
		uint32_t height;
		uint32_t mipLevels;
		uint32_t arrayLayers;

		TextureFormat format;

		SampleCount samples;

		bool ownsImage = true;

		vk::Image GetVkImage() const
		{
			if(ownsImage)
				return *std::get<vk::raii::Image>(image);
			else
				return std::get<vk::Image>(image);
		}

		uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

        explicit Impl(Image& p, const core::gpu::Device* device,
			const SImageCreateInfo& info);

		explicit Impl(Image& p, const core::gpu::Device* device,
					  vk::Image swapchainImage, uint32_t width, uint32_t height,
					  TextureFormat format);

		~Impl();

		void CreateView(const SImageViewCreateInfo& info);

		void TransitionLayout(CommandBuffer& commandBuffer, ImageLayout oldLayout,
			ImageLayout newLayout, uint32_t mipLevels);

		void CopyFromBuffer(CommandBuffer& commandBuffer, Buffer& buffer,
			uint32_t width, uint32_t height);

		void GenerateMipmaps(CommandBuffer& commandBuffer, uint32_t width,
			uint32_t height, uint32_t mipLevels);
	};
}

#endif //VRAKTAL_CORE_GPU_VULKAN_IMAGE_H