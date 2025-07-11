#ifndef VRAKTAL_VK_SWAPCHAIN_H
#define VRAKTAL_VK_SWAPCHAIN_H
#pragma once

#include "../src/vkb/VkBootstrap.h"

#include <rhi/swapchain.h>
#include <vector>
#include <vma/vk_mem_alloc.h>

namespace vk
{
	class VulkanSwapchain : public rhi::Swapchain
	{
    public:
		explicit VulkanSwapchain();
		~VulkanSwapchain() noexcept;

		bool Create(uint32_t width, uint32_t height) override;
		void Destroy() override;
		void Resize(uint32_t width, uint32_t height) override;
		void Present() override;

    private:
        VkDevice _device;
        VkSwapchainKHR _swapchain;
        std::vector<VkImage> _swapchainImages;
        std::vector<VkImageView> _swapchainImageViews;
        VkFormat _swapchainImageFormat;
        VkExtent2D _swapchainExtent;

        struct ImageResource {
            VkImage image;
            VkImageView imageView;
            VkFormat imageFormat;
            VkExtent3D imageExtent;
            VmaAllocation allocation;
        };

        ImageResource _drawImage;
        ImageResource _depthImage;
	};
}

#endif //VRAKTAL_VK_SWAPCHAIN_H
