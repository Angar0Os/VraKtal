#ifndef VRAKTAL_CORE_GPU_SWAPCHAIN_H
#define VRAKTAL_CORE_GPU_SWAPCHAIN_H
#pragma once

#include <memory>
#include <vector>
#include <cstdint>
#include <core/enum.h>

namespace core::gpu
{
	class Device;
	class Image;
	class Swapchain;

	struct SwapchainCreateInfo
	{
		void* surface = nullptr;
		uint32_t width = 0;
		uint32_t height = 0;
		TextureFormat preferredFormat = TextureFormat::RGBA8_SRGB;
		PresentMode presentMode = PresentMode::Fifo;
		uint32_t minImageCount = 2;
		const Swapchain* oldSwapchain;
	};

	class Swapchain
	{
	private:
		struct Impl;
		std::unique_ptr<Impl> m_impl;

	public:
		Swapchain(const core::gpu::Device* device, const SwapchainCreateInfo& info);
		~Swapchain();

		Swapchain(const Swapchain&) = delete;
		Swapchain& operator=(const Swapchain&) = delete;

		Swapchain(Swapchain&&) noexcept;
		Swapchain& operator=(Swapchain&&) noexcept;

		uint32_t AcquireNextImage(void* semaphore, uint64_t timeout = UINT64_MAX);

		Impl& GetImpl() const;
	};
}

#endif // VRAKTAL_CORE_GPU_SWAPCHAIN_H