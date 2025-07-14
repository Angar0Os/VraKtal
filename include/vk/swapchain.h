#ifndef VRAKTAL_VK_SWAPCHAIN_H
#define VRAKTAL_VK_SWAPCHAIN_H
#pragma once

#include "../src/vkb/VkBootstrap.h"

#include <rhi/swapchain.h>

#include <vk/image.h>
#include <vma/vk_mem_alloc.h>
#include <vector>

#include <queue>
#include <functional>


namespace vk
{
	struct DeletionQueue
	{
		std::deque<std::function<void()>> deletors;

		void push_function(std::function<void()>&& function)
		{
			deletors.push_back(function);
		}

		void flush()
		{
			for (auto it = deletors.rbegin(); it != deletors.rend(); ++it)
			{
				(*it)();
			}

			deletors.clear();
		}
	};  // TODO : Move to Device ? Or command ?

	class VulkanSwapchain : public rhi::Swapchain
	{
    public: 
		VulkanSwapchain() = default;

		VulkanSwapchain(VkExtent2D size, VkPhysicalDevice chosenGPU, VkDevice device, VkSurfaceKHR surface)
			: _windowExtent(size), _chosenGPU(chosenGPU), _device(device), _surface(surface) {
		}

		~VulkanSwapchain() noexcept;

		void Create(uint32_t width, uint32_t height) override;
		void Destroy() override;
		void Resize(uint32_t width, uint32_t height) override;
		void Present() override;
		void Init() override;

		VkSwapchainKHR GetSwapchain() { return _swapchain; };
		VkFormat GetSwapchainImageFormat() { return _swapchainImageFormat; };
		VkExtent2D GetSwapchainExtent() { return _swapchainExtent; };
		std::vector<VkImage> GetSwapchainImages() { return _swapchainImages; };
		std::vector<VkImageView> GetSwapchainImagesViews() { return _swapchainImageViews; };

    private:
		VkExtent2D _windowExtent;
		VkPhysicalDevice _chosenGPU;
		VkDevice _device;
		VkSurfaceKHR _surface;

		DeletionQueue _mainDeletionQueue; // TODO : Move to Device ? Or command ?

		AllocatedImage _drawImage;
		AllocatedImage _depthImage;

		VulkanImage _drawImageHandler;

		VkSwapchainKHR _swapchain;
		VkFormat _swapchainImageFormat;
		VkExtent2D _swapchainExtent;
		std::vector<VkImage> _swapchainImages;
		std::vector<VkImageView> _swapchainImageViews;
	};
}

#endif //VRAKTAL_VK_SWAPCHAIN_H
