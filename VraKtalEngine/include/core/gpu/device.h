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
		class CommandPool;
		class DescriptorPool;
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

			CommandPool& GetCommandPool() const;
			DescriptorPool& GetDescriptorPool() const;

			uint32_t AcquireNextImage(uint32_t frameIndex);

			void Present(uint32_t imageIndex, uint32_t frameIndex);
			void Cleanup();

			void WaitIdle();

			core::gpu::Image* GetSwapchainImage(uint32_t imageIndex) const;

			static constexpr uint32_t s_FRAMES_IN_FLIGHT = 2;

			ImguiContext* GetImGuiContext();

			void BeginFrame(uint32_t frameIndex);
			std::pair<uint32_t, uint32_t> GetSwapchainExtent() const;
			
			void TransitionImageForPresent(uint32_t frameIndex, uint32_t imageIndex);
			
			bool NeedsResize() const;
			void ClearResizeFlag();
			void RecreateSwapchain();

		};
	}
}

#endif //VRAKTAL_CORE_GPU_DEVICE_H