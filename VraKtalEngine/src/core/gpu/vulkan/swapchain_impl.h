#ifndef VRAKTAL_CORE_GPU_VULKAN_SWAPCHAIN_H
#define VRAKTAL_CORE_GPU_VULKAN_SWAPCHAIN_H
#pragma once

#include <core/gpu/swapchain.h>
#include <vulkan/vulkan_raii.hpp>

namespace core::gpu
{
	class Image;

	struct Swapchain::Impl
	{
		Swapchain& parent;
		const core::gpu::Device* device;

		vk::raii::SwapchainKHR swapchain;

		std::vector<std::unique_ptr<Image>> images;
		std::vector<vk::raii::ImageView> imageViews;

		vk::Format format;
		vk::Extent2D extent;

		vk::SurfaceFormatKHR ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats,
			TextureFormat preferredFormat);
		vk::PresentModeKHR ChoosePresentMode(const std::vector<vk::PresentModeKHR>& availableModes,
			PresentMode preferredMode);
		vk::Extent2D ChooseExtent(const vk::SurfaceCapabilitiesKHR& capabilities,
			uint32_t width, uint32_t height);

        explicit Impl(Swapchain& p, const core::gpu::Device* dev, const SwapchainCreateInfo& info);
		~Impl();

		uint32_t AcquireNextImage(vk::Semaphore semaphore, uint64_t timeout);
	};
}

#endif // VRAKTAL_CORE_GPU_VULKAN_SWAPCHAIN_H