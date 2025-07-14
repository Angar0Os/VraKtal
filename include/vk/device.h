#ifndef VRAKTAL_VK_DEVICE_H
#define VRAKTAL_VK_DEVICE_H
#pragma once

#include "../src/vkb/VkBootstrap.h"
#include <vma/vk_mem_alloc.h>

#include <rhi/device.h>

#include <vk/commandBuffer.h>

namespace vk
{
	class VulkanDevice : public rhi::Device
	{
	public:
		VulkanDevice() = default;

		VulkanDevice(vkb::Instance instance, VkSurfaceKHR surface)
			: _instance(instance), _surface(surface) {
		}

		void Initialize() override;
		void Shutdown() override;

		VkDevice GetVkDevice() const { return _device; }
		VkPhysicalDevice GetVkPhysicalDevice() const { return _chosenGPU; }
		VmaAllocator GetAllocator() const { return _allocator; }
		VkQueue GetGraphicsQueue() const { return _graphicsQueue; }
		uint32_t GetGraphicsQueueFamily() const { return _graphicsQueueFamily; }
		VulkanCommandBuffer& GetCommandBuffer() { return _commandBuffer; }
	private:
		vkb::Instance _instance;
		VkSurfaceKHR _surface;

		VkDevice _device = VK_NULL_HANDLE;
		VkPhysicalDevice _chosenGPU = VK_NULL_HANDLE;
		VkQueue _graphicsQueue = VK_NULL_HANDLE;
		uint32_t _graphicsQueueFamily = 0;
		VmaAllocator _allocator = VK_NULL_HANDLE;
		VulkanCommandBuffer _commandBuffer;
	};
}

#endif //VRAKTAL_VK_DEVICE_H
