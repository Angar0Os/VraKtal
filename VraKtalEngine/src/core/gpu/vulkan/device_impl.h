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
#include <core/gpu/sampler.h>
#include <core/gpu/image.h>
#include <core/gpu/commandBuffer.h>
#include <core/gpu/texture.h>
#include <core/gpu/descriptorPool.h>
#include <core/gpu/commandPool.h>
#include <core/gpu/swapchain.h>
#include <core/gpu/pipeline.h>

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
        const core::gpu::Device*			parent = nullptr;

		vk::raii::Context					context;
		vk::raii::Instance					instance = nullptr;
		vk::raii::DebugUtilsMessengerEXT	debugMessenger = nullptr;
		vk::raii::SurfaceKHR				surface = nullptr;
		vk::raii::Device					device = nullptr;
		vk::raii::PhysicalDevice			physicalDevice = nullptr;
		vk::raii::Queue						graphicsQueue = nullptr;
		uint32_t							queueIndex = ~0;

		std::unique_ptr<CommandPool>			commandPool;
		std::unique_ptr<DescriptorSetLayout>	descriptorSetLayout;
		std::unique_ptr<DescriptorPool>			descriptorPool;
		std::vector<vk::raii::DescriptorSet*>	descriptorSets;

		std::vector<std::unique_ptr<Buffer>> uniformBuffers;

		std::unique_ptr<Sampler> textureSampler;

		std::unique_ptr<Texture> defaultWhiteTexture;
		std::unique_ptr<Texture> defaultBlackTexture;
		std::unique_ptr<Texture> defaultNormalTexture;

		std::unique_ptr<Texture> albedoTexture;
		std::unique_ptr<Texture> normalTexture;
		std::unique_ptr<Texture> metallicTexture;
		std::unique_ptr<Texture> roughnessTexture;
		std::unique_ptr<Texture> aoTexture;
		std::unique_ptr<Texture> emissiveTexture;

		std::unique_ptr<Image>	colorImage;
		std::unique_ptr<Image>	depthImage;

		std::unique_ptr<Swapchain>	swapchain;
		std::unique_ptr<Pipeline>	graphicsPipeline;

		std::vector<CommandBuffer>	commandBuffers;

		std::vector<vk::raii::Semaphore>	imageAvailable;
		std::vector<vk::raii::Semaphore>	renderFinished;
		std::vector<vk::raii::Fence>		inFlightFences;
		std::vector<const vk::raii::Fence*> imagesInFlight;
		std::vector<std::unique_ptr<vk::raii::CommandBuffer>> tempCmdBufs;

		const Window& m_window;

		std::vector<char> ReadFile(const std::string& filename);

		explicit Impl(Window& window, const core::gpu::Device* parent);
		~Impl();

		void Initialize();

		void CreateInstance();
		void SetupDebugMessenger();
		void CreateSurface();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSwapchain();
		void CreateGraphicsPipeline();
		void RecreateSwapchain();

		void CreateDescriptorSetLayout();
		void CreateCommandPool();
		void CreateDescriptorPool();
		void AllocateDescriptorSets();
		void CreateUniformBuffers();
		void CreateSamplers();
		void CreateDefaultTextures();
		void LoadMaterialTextures();
		void CreateColorImage();
		void CreateDepthImage();
		void CreateDescriptorSets();
		void CreateCommandBuffers();
		void CreateSyncObjects();

		void BeginFrame(uint32_t frameIndex);
		uint32_t AcquireNextImage(uint32_t frameIndex);
		void* GetImageAvailableSemaphore(uint32_t frameIndex) const;
		void* GetRenderFinishedSemaphore(uint32_t imageIndex) const;
		void* GetInFlightFence(uint32_t frameIndex) const;
		void Present(uint32_t imageIndex);
		void Cleanup();

		Buffer* GetUniformBuffer(uint32_t frameIndex) const;

		const Swapchain* GetSwapchain() const;
		const core::gpu::Image* GetSwapchainImage(uint32_t imageIndex) const;
		const core::gpu::Image* GetColorImage() const;
		const core::gpu::Image* GetDepthImage() const;

        const core::gpu::Pipeline* GetGraphicsPipeline() const;

		void WaitIdle();

		void TransitionImageForPresent(uint32_t frameIndex, uint32_t imageIndex);

		void UpdateDescriptorWithTLAS(uint32_t frameIndex, const core::gpu::AccelerationStructure* tlasHandle);

		std::vector<const char*> requiredDeviceExtension = {
			vk::KHRSwapchainExtensionName,
			vk::KHRSpirv14ExtensionName,
			vk::KHRSynchronization2ExtensionName,
			vk::KHRCreateRenderpass2ExtensionName,

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