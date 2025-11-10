#ifndef VRAKTAL_CORE_GPU_DEVICE_H
#define VRAKTAL_CORE_GPU_DEVICE_H
#pragma once

#include <memory>
#include <cstdint>

namespace core
{
	class Window;

	namespace gpu
	{
		class Device
		{
		private:
			struct Impl;
			std::unique_ptr<Impl> m_impl;
		public:
			explicit Device(Window& window);
			~Device();

			Impl& GetImpl();

			void* GetDeviceHandle() const;
			void* GetPhysicalDeviceHandle() const;
			void* GetGraphicsQueueHandle() const;
			void* GetCommandPoolHandle() const;
			void* GetSwapchainHandle() const;
			uint32_t GetSwapchainImageCount() const;

			void* GetSwapchainImageViewHandle(uint32_t index) const;
			void* GetSwapchainImageHandle(uint32_t index) const;
		};
	}
}

#endif //VRAKTAL_CORE_GPU_DEVICE_H