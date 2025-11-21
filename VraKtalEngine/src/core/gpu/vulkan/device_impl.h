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
#include <core/gpu/commandPool.h>
#include <core/gpu/swapchain.h>
#include <core/gpu/pipeline.h>

#include <glm/glm.hpp>

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

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

		std::unique_ptr<CommandPool> commandPool;

		std::unique_ptr<DescriptorSetLayout> descriptorSetLayout;
		std::unique_ptr<DescriptorSetLayout> shadowDescriptorSetLayout;

		std::unique_ptr<DescriptorPool> descriptorPool;

		std::vector<vk::raii::DescriptorSet*> descriptorSets;
		std::vector<vk::raii::DescriptorSet*> shadowDescriptorSets;

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

		std::unique_ptr<Image> colorImage;
		std::unique_ptr<Image> shadowMapImage;

		std::unique_ptr<Swapchain> swapchain;
		std::unique_ptr<Pipeline> graphicsPipeline;
		std::unique_ptr<Pipeline> shadowPipeline;

		std::vector<CommandBuffer> commandBuffers;

		std::vector<vk::raii::Semaphore> imageAvailable;
		std::vector<vk::raii::Semaphore> renderFinished;
		std::vector<vk::raii::Fence> inFlightFences;
		std::vector<const vk::raii::Fence*> imagesInFlight;  
		std::vector<std::unique_ptr<vk::raii::CommandBuffer>> tempCmdBufs;

		const Window& m_window;

		std::vector<char> ReadFile(const std::string& filename);
	public:
		explicit Impl(Window& window);
		~Impl();

		void CreateInstance();
		void SetupDebugMessenger();
		void CreateSurface();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSwapchain();
		void CreateGraphicsPipeline();
		void CreateShadowPipeline();
		void RecreateSwapchain();

		void CreateDescriptorSetLayout();
		void CreateShadowDescriptorSetLayout();
		void CreateCommandPool();
		void CreateDescriptorPool();
		void AllocateDescriptorSets();
		void CreateUniformBuffers();
		void CreateSamplers();
		void CreateDefaultTextures();
		void LoadMaterialTextures();
		void CreateShadowMap();
		void CreateColorImage();
		void CreateDescriptorSets();
		void CreateShadowDescriptorSets();
		void CreateCommandBuffers();
		void CreateSyncObjects();

		void BeginFrame(uint32_t frameIndex);
		uint32_t AcquireNextImage(uint32_t frameIndex);
		void* GetImageAvailableSemaphore(uint32_t frameIndex) const;
		void* GetRenderFinishedSemaphore(uint32_t imageIndex) const;
		void* GetInFlightFence(uint32_t frameIndex) const;
		void Present(uint32_t imageIndex);
		void Cleanup();

		void* GetCommandPool() const;
		void* GetGraphicsQueue() const;
		void* GetPipeline() const;
		void* GetPipelineLayout() const;
		void* GetDescriptorSet(uint32_t frameIndex) const;
		uint32_t GetSwapchainWidth() const;
		uint32_t GetSwapchainHeight() const;
		void* GetSwapchainImageView(uint32_t index) const;
		void* GetDepthImageView() const;
		void* GetColorImageView() const;
		Buffer* GetUniformBuffer(uint32_t frameIndex) const;

		void* GetPhysicalDevice() const;
		void* GetHandle() const;

		void* GetSwapchainImage(uint32_t imageIndex) const;
		void* GetColorImage() const;
		void* GetDepthImage() const;

		void WaitIdle();

		void TransitionImageForPresent(uint32_t frameIndex, uint32_t imageIndex);

		std::vector<const char*> requiredDeviceExtension = {
			vk::KHRSwapchainExtensionName,
			vk::KHRSpirv14ExtensionName,
			vk::KHRSynchronization2ExtensionName,
			vk::KHRCreateRenderpass2ExtensionName
		};
	};
}

#endif //VRAKTAL_CORE_GPU_VULKAN_DEVICE_H