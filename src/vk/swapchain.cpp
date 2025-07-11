#include <vk/swapchain.h>
#include <GLFW/glfw3.h>

#include "../src/vkb/VkBootstrap.h"

using namespace vk;

VulkanSwapchain::VulkanSwapchain()
{

}

VulkanSwapchain::~VulkanSwapchain()
{

}

void VulkanSwapchain::Create(uint32_t width, uint32_t height)
{
	vkb::SwapchainBuilder swapchainBuilder{ _chosenGPU, _device, _surface };

	_swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

	vkb::Swapchain vkbSwapchain = swapchainBuilder
		.set_desired_format(VkSurfaceFormatKHR{ .format = _swapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.set_desired_extent(width, height)
		.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		.build()
		.value();

	_swapchainExtent = vkbSwapchain.extent;
	_swapchain = vkbSwapchain.swapchain;
	_swapchainImages = vkbSwapchain.get_images().value();
	_swapchainImageViews = vkbSwapchain.get_image_views().value();
}

void VulkanSwapchain::Destroy()
{

}

void VulkanSwapchain::Resize(uint32_t width, uint32_t height)
{

}

void VulkanSwapchain::Present()
{

}


