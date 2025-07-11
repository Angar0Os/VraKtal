#ifndef VRAKTAL_RHI_DEVICE_H
#define VRAKTAL_RHI_DEVICE_H
#pragma once

namespace rhi
{
	class Device
	{
	public:
		virtual ~Device() noexcept = default;
		virtual void Initialize() = 0;
		virtual void Shutdown() = 0;
	};
}

#endif //VRAKTAL_RHI_DEVICE_H
