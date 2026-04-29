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

#include <core/gpu/accelerationStructure.h>
#include <core/gpu/device.h>
#include <core/window.h>
#include <core/gpu/descriptorSetLayout.h>
#include <core/gpu/buffer.h>
#include <core/gpu/image.h>
#include <core/gpu/commandBuffer.h>
#include <core/gpu/texture.h>
#include <core/gpu/descriptorPool.h>
#include <core/gpu/commandPool.h>
#include <core/gpu/pipeline.h>

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

namespace core::gpu
{
	struct Device::Impl
	{
		struct FrameSync
		{
			vk::raii::Fence     inFlightFence;
			vk::raii::Semaphore imageAvailable;
			vk::raii::Semaphore renderFinished;
		};

		const Device* parent = nullptr;

		vk::raii::Context					context;
		vk::raii::Instance					instance		= nullptr;
		vk::raii::DebugUtilsMessengerEXT	debugMessenger	= nullptr;
		vk::raii::SurfaceKHR				surface			= nullptr;
		vk::raii::Device					device			= nullptr;
		vk::raii::PhysicalDevice			physicalDevice	= nullptr;
		vk::raii::Queue						graphicsQueue	= nullptr;
		uint32_t							queueIndex		= ~0;
		bool								needsResize		= false;

		std::unique_ptr<CommandPool>			commandPool;
		std::unique_ptr<DescriptorSetLayout>	descriptorSetLayout;
		std::unique_ptr<DescriptorPool>			descriptorPool;

		std::vector<CommandBuffer>	commandBuffers;

		vk::raii::SwapchainKHR				swapchain = nullptr;
		std::vector<vk::raii::ImageView>    swapchainImageViews;
		vk::Extent2D						swapchainExtent;
		std::vector<std::unique_ptr<Image>>	swapchainImages;
		vk::Format							swapchainImageFormat;

		std::vector<FrameSync>              frameSyncObjects;
		std::vector<std::unique_ptr<vk::raii::CommandBuffer>> tempCmdBufs;

		const Window& m_window;

		explicit Impl(Window& window, const core::gpu::Device* parent);
		~Impl();

		void Initialize();

		void CreateInstance();
		void SetupDebugMessenger();
		void CreateSurface();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSwapchain();
		void RecreateSwapchain();

		void CreateCommandPool();
		void CreateDescriptorPool();

		void CreateSyncObjects();

		vk::SurfaceFormatKHR	ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats, TextureFormat preferredFormat);
		vk::PresentModeKHR		ChoosePresentMode(const std::vector<vk::PresentModeKHR>& availableModes, PresentMode preferredMode);
		vk::Extent2D			ChooseExtent(const vk::SurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height);

		void TransitionImageForPresent(uint32_t frameIndex, uint32_t imageIndex);

		std::vector<const char*> requiredDeviceExtension = {
			vk::KHRSwapchainExtensionName,
			vk::KHRSpirv14ExtensionName,
			vk::KHRSynchronization2ExtensionName,
			vk::KHRCreateRenderpass2ExtensionName,
			vk::KHRComputeShaderDerivativesExtensionName,

			VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
			VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
			VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
			VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
			VK_KHR_RAY_QUERY_EXTENSION_NAME,
			VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
			VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME
		};
	};
}

#endif //VRAKTAL_CORE_GPU_VULKAN_DEVICE_H