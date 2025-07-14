#ifndef VRAKTAL_RHI_SYNC_H
#define VRAKTAL_RHI_SYNC_H
#pragma once

#include <vulkan/vulkan.h>

namespace rhi
{
	class Sync
	{
		virtual VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags) = 0;
		virtual VkSemaphoreCreateInfo SemaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0) = 0;
	};
}

#endif //VRAKTAL_RHI_SYNC_H
