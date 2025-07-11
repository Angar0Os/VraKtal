#ifndef VRAKTAL_RHI_SWAPCHAIN_H
#define VRAKTAL_RHI_SWAPCHAIN_H
#pragma once

#include <iostream>

namespace rhi
{
	class Swapchain
	{
	public:
		virtual ~Swapchain() noexcept = default;

		virtual bool Create(uint32_t width, uint32_t height) = 0;
		virtual void Destroy() = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void Present() = 0;
	};
}

#endif // VRAKTAL_RHI_SWAPCHAIN_H
