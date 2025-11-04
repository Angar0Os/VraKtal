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
		};
	}
}

#endif //VRAKTAL_CORE_GPU_DEVICE_H