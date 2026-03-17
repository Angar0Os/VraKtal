#ifndef VRAKTAL_CORE_GPU_VULKAN_IMAGE_H
#define VRAKTAL_CORE_GPU_VULKAN_IMAGE_H
#pragma once

#include <core/gpu/image.h>
#include <vulkan/vulkan_raii.hpp>

namespace core::gpu
{
	struct SPredefinedImageCreateInfo
	{
		vk::Image image = nullptr;
		vk::Extent2D extent = { 0, 0 };
		vk::ImageAspectFlags aspectFlags = {};
		vk::Format format = vk::Format::eUndefined;
	};

	struct Image::Impl
	{
		vk::Image				image = nullptr;
		vk::raii::Image			raiiImage = nullptr;
		vk::raii::DeviceMemory	memory = nullptr;
		vk::raii::ImageView		view = nullptr;
		vk::ImageLayout			currentLayout = vk::ImageLayout::eUndefined;
		vk::Extent2D			extent = { 0, 0 };  

		TextureFormat format;
		SampleCount samples;

		uint32_t FindMemoryType(const core::gpu::Device& device, uint32_t typeFilter, vk::MemoryPropertyFlags properties);

		explicit Impl(const core::gpu::Device* device, const SImageCreateInfo& info);
		explicit Impl(const core::gpu::Device* device, const SPredefinedImageCreateInfo& info);

		~Impl();

		/* Dans commamdBuffer ducoup
		void TransitionLayout(CommandBuffer& commandBuffer, ImageLayout oldLayout,
			ImageLayout newLayout, uint32_t mipLevels);

		void CopyFromBuffer(CommandBuffer& commandBuffer, Buffer& buffer,
			uint32_t width, uint32_t height);

		void GenerateMipmaps(CommandBuffer& commandBuffer, uint32_t width,
			uint32_t height, uint32_t mipLevels);
		*/
	};
}

#endif //VRAKTAL_CORE_GPU_VULKAN_IMAGE_H