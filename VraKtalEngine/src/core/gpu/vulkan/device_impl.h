#ifndef VRAKTAL_CORE_GPU_VULKAN_DEVICE_H
#define VRAKTAL_CORE_GPU_VULKAN_DEVICE_H
#pragma once

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include <core/gpu/device.h>

// Enable validation layers in debug builds
#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

namespace core::gpu
{
	struct Device::Impl
	{
	private:
		vk::raii::Context					context;
		vk::raii::Instance					instance = nullptr;
		vk::raii::DebugUtilsMessengerEXT	debugMessenger = nullptr;
		vk::raii::SurfaceKHR				surface = nullptr;
		vk::raii::Device					device = nullptr;
		vk::raii::PhysicalDevice			physicalDevice = nullptr;
		vk::raii::Queue						graphicsQueue = nullptr;
		uint32_t							queueIndex = ~0;

		vk::raii::SwapchainKHR				swapChain = nullptr;
		std::vector<vk::Image>				swapChainImages;
		std::vector<vk::raii::ImageView>	swapChainImageViews;
		vk::SurfaceFormatKHR				swapChainSurfaceFormat;
		vk::Extent2D						swapChainExtent;

		const Window& m_window;

		vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
	public:
		explicit Impl(const Window& window);
		~Impl();

		void CreateInstance();
		void SetupDebugMessenger();
		void CreateSurface();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSwapchain();
		void CreateImageViews();

		std::vector<const char*> requiredDeviceExtension = {
			vk::KHRSwapchainExtensionName,
			vk::KHRSpirv14ExtensionName,
			vk::KHRSynchronization2ExtensionName,
			vk::KHRCreateRenderpass2ExtensionName
		};
	};
}

#endif //ifndef VRAKTAL_CORE_GPU_DEVICE_H
