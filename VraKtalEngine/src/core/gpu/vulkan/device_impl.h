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
#include <core/window.h>
#include <core/gpu/descriptorSet.h>
#include <core/gpu/descriptorSetLayout.h>
#include <core/gpu/buffer.h>
#include <core/gpu/sampler.h>
#include <core/gpu/image.h>
#include <core/gpu/commandBuffer.h>
#include <core/gpu/texture.h>
#include <core/gpu/descriptorPool.h>

// Enable validation layers in debug builds
#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

// Number of frames the application will use for in-flight frames
constexpr int MAX_FRAMES_IN_FLIGHT = 2;

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

		// Note : We will do an abstract of commandPools
		vk::raii::CommandPool commandPool = nullptr;

		std::unique_ptr<DescriptorSetLayout> descriptorSetLayout;
		std::unique_ptr<DescriptorPool> descriptorPool;
		std::vector<void*> descriptorSets;
		std::vector<std::unique_ptr<Buffer>> uniformBuffers;

		std::unique_ptr<Sampler> textureSampler;
		std::unique_ptr<Sampler> shadowSampler;

		std::unique_ptr<Texture> defaultWhiteTexture;
		std::unique_ptr<Texture> defaultBlackTexture;
		std::unique_ptr<Texture> defaultNormalTexture;

		std::unique_ptr<Texture> albedoTexture;
		std::unique_ptr<Texture> normalTexture;
		std::unique_ptr<Texture> metallicTexture;
		std::unique_ptr<Texture> roughnessTexture;
		std::unique_ptr<Texture> aoTexture;
		std::unique_ptr<Texture> emissiveTexture;

		std::unique_ptr<Image> shadowMapImage;

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

		void CreateDescriptorSetLayout();
		void CreateDescriptorPool();
		void AllocateDescriptorSets();
		void CreateUniformBuffers();
		void CreateSamplers();
		void CreateDefaultTextures();
		void LoadMaterialTextures();
		void CreateShadowMap();
		void CreateDescriptorSets();

		std::vector<const char*> requiredDeviceExtension = {
			vk::KHRSwapchainExtensionName,
			vk::KHRSpirv14ExtensionName,
			vk::KHRSynchronization2ExtensionName,
			vk::KHRCreateRenderpass2ExtensionName
		};
	};
}

#endif //ifndef VRAKTAL_CORE_GPU_DEVICE_H
