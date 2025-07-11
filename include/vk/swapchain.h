#ifndef VRAKTAL_VK_SWAPCHAIN_H
#define VRAKTAL_VK_SWAPCHAIN_H
#pragma once

#include "../src/vkb/VkBootstrap.h"

#include <rhi/swapchain.h>
#include <vector>

namespace vk
{
	class VulkanSwapchain : public rhi::Swapchain
	{
    public:
		VulkanSwapchain(VkExtent2D size, VkPhysicalDevice chosenGPU, VkDevice device, VkSurfaceKHR surface)
			: _windowExtent(size), _chosenGPU(chosenGPU), _device(device), _surface(surface) {};

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

		VkSwapchainKHR _swapchain;
		VkFormat _swapchainImageFormat;
		VkExtent2D _swapchainExtent;
		std::vector<VkImage> _swapchainImages;
		std::vector<VkImageView> _swapchainImageViews;
	};
}

#endif //VRAKTAL_VK_SWAPCHAIN_H
