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
		class AccelerationStructure;
		class Buffer;
		class Image;
        class Pipeline;
		class ImguiContext;

		class Device
		{
		private:
			struct Impl;
			std::unique_ptr<Impl> m_impl;
			ImguiContext* m_imGuiContext;
		public:
			explicit Device(Window& window);
			~Device();

			Impl& GetImpl() const;

			void BeginFrame(uint32_t frameIndex);
			uint32_t AcquireNextImage(uint32_t frameIndex);
			void* GetImageAvailableSemaphore(uint32_t frameIndex) const;
			void* GetRenderFinishedSemaphore(uint32_t imageIndex) const;
			void* GetInFlightFence(uint32_t frameIndex) const;
			void TransitionImageForPresent(uint32_t frameIndex, uint32_t imageIndex);
			void Present(uint32_t imageIndex);
			void Cleanup();

			std::pair<uint32_t, uint32_t> GetSwapchainExtent() const;
			

			const core::gpu::Image* GetSwapchainImage(uint32_t imageIndex) const;

			void RecreateSwapchain();
			void WaitIdle();
			static constexpr uint32_t s_FRAMES_IN_FLIGHT = 2;

			ImguiContext* GetImGuiContext();
		};
	}
}

#endif //VRAKTAL_CORE_GPU_DEVICE_H