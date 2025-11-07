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
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 texCoord;
		glm::vec3 normal;

		static vk::VertexInputBindingDescription GetBindingDescription()
		{
			return { 0, sizeof(Vertex), vk::VertexInputRate::eVertex };
		}

		static std::array<vk::VertexInputAttributeDescription, 4> GetAttributeDescriptions()
		{
			return
			{
				vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)),
				vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)),
				vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord)),
				vk::VertexInputAttributeDescription(3, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal))
			};
		}

		bool operator==(const Vertex& other) const
		{
			return pos == other.pos && color == other.color && texCoord == other.texCoord;
		}
	};

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
		
		std::vector<void*> descriptorSets;
		std::vector<void*> shadowDescriptorSets;

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

		std::unique_ptr<Swapchain> swapchain;
		std::unique_ptr<Pipeline> graphicsPipeline;
		std::unique_ptr<Pipeline> shadowPipeline;

		const Window& m_window;

		std::vector<char> ReadFile(const std::string& filename);
	public:
		explicit Impl(const Window& window);
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
		void CreateDescriptorSets();
		void CreateShadowDescriptorSets();

		std::vector<const char*> requiredDeviceExtension = {
			vk::KHRSwapchainExtensionName,
			vk::KHRSpirv14ExtensionName,
			vk::KHRSynchronization2ExtensionName,
			vk::KHRCreateRenderpass2ExtensionName
		};
	};
}

#endif //ifndef VRAKTAL_CORE_GPU_DEVICE_H
