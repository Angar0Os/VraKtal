#ifndef VRAKTAL_CORE_GPU_DEVICE_H
#define VRAKTAL_CORE_GPU_DEVICE_H
#pragma once

#include <memory>
#include <cstdint>

#include <glm/glm.hpp>

namespace core
{
	class Window;

	namespace gpu
	{
		class Buffer;

		constexpr int MAX_LIGHTS = 10;

		// Note : We will maybe move this, but this is here to make uniform buffers work properly.
		struct UniformBufferObject
		{
			alignas(16) glm::mat4 model;
			alignas(16) glm::mat4 view;
			alignas(16) glm::mat4 proj;
			alignas(16) glm::mat4 lightSpaceMatrix;
			alignas(16) glm::vec3 viewPos;

			struct LightData
			{
				alignas(16) glm::vec3 position;
				alignas(16) glm::vec3 color;
				alignas(4)  float intensity;
				alignas(4)	int enabled;
				alignas(4)	int type;
				alignas(4)  int _padding; // Note : Simplement pour s'aligner à 48bytes
			};

			LightData lights[MAX_LIGHTS];
			alignas(4) int numLights;

			alignas(16) glm::vec3 albedo;
			alignas(4)  float metallic;
			alignas(4)  float roughness;
			alignas(4)  float ao;
			alignas(16) glm::vec3 emissive;

			alignas(4) uint32_t useAlbedoMap;
			alignas(4) uint32_t useNormalMap;
			alignas(4) uint32_t useMetallicMap;
			alignas(4) uint32_t useRoughnessMap;
			alignas(4) uint32_t useAOMap;
			alignas(4) uint32_t useEmissiveMap;
		};


		class Device
		{
		private:
			struct Impl;
			std::unique_ptr<Impl> m_impl;
		public:
			explicit Device(Window& window);
			~Device();

			Impl& GetImpl();

			void BeginFrame(uint32_t frameIndex);
			uint32_t AcquireNextImage(uint32_t frameIndex);
			void* GetImageAvailableSemaphore(uint32_t frameIndex) const;
			void* GetRenderFinishedSemaphore(uint32_t imageIndex) const;
			void* GetInFlightFence(uint32_t frameIndex) const;
			void TransitionImageForPresent(uint32_t frameIndex, uint32_t imageIndex);
			void Present(uint32_t imageIndex);
			void Cleanup();

			void* GetHandle() const;
			void* GetCommandPool() const;
			void* GetGraphicsQueue() const;
			void* GetPipeline() const;
			void* GetPipelineLayout() const;
			void* GetDescriptorSet(uint32_t frameIndex) const;
			core::gpu::Buffer* GetUniformBuffer(uint32_t frameIndex);

			uint32_t GetSwapchainWidth() const;
			uint32_t GetSwapchainHeight() const;
			void* GetSwapchainImageView(uint32_t imageIndex) const;
			void* GetDepthImageView() const;
			void* GetColorImageView() const;
			void* GetPhysicalDevice() const;
			void* GetSwapchainImage(uint32_t imageIndex) const;
			void* GetColorImage() const;
			void* GetDepthImage() const;

			void WaitIdle();

			static constexpr uint32_t FRAMES_IN_FLIGHT = 2;
		};
	}
}

#endif //VRAKTAL_CORE_GPU_DEVICE_H