#include <vk/swapchain.h>
#include <vk/commandBuffer.h>
#include <vk/image.h>

#include <GLFW/glfw3.h>

#include "../src/vkb/VkBootstrap.h"

#include <GLFW/glfw3.h>

using namespace vk;

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

void VulkanSwapchain::Init()
{
	Create(_windowExtent.width, _windowExtent.height);

	VkExtent3D drawImageExtent = {
		_windowExtent.width,
		_windowExtent.height,
		1
	};

	_drawImage->imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
	_drawImage->imageExtent = drawImageExtent;

	VkImageUsageFlags drawImageUsages{};
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	VkImageCreateInfo rimg_info = _drawImageHandler->CreateInfo(_drawImage->imageFormat, drawImageUsages, drawImageExtent);

	VmaAllocationCreateInfo rimg_allocinfo = {};
	rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vmaCreateImage(_drawImageHandler->GetAllocator(), &rimg_info, &rimg_allocinfo, &_drawImage->image, &_drawImage->allocation, nullptr);

	VkImageViewCreateInfo rview_info = _drawImageHandler->CreateViewInfo(_drawImage->imageFormat, _drawImage->image, VK_IMAGE_ASPECT_COLOR_BIT);

	vkCreateImageView(_device, &rview_info, nullptr, &_drawImage->imageView);

	_depthImage->imageFormat = VK_FORMAT_D32_SFLOAT;
	_depthImage->imageExtent = drawImageExtent;
	VkImageUsageFlags depthImageUsages{};
	depthImageUsages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	VkImageCreateInfo dimg_info = _drawImageHandler->CreateInfo(_depthImage->imageFormat, depthImageUsages, drawImageExtent);

	vmaCreateImage(_drawImageHandler->GetAllocator(), &dimg_info, &rimg_allocinfo, &_depthImage->image, &_depthImage->allocation, nullptr);
	VkImageViewCreateInfo dview_info = _drawImageHandler->CreateViewInfo(_depthImage->imageFormat, _depthImage->image, VK_IMAGE_ASPECT_DEPTH_BIT);

	vkCreateImageView(_device, &dview_info, nullptr, &_depthImage->imageView);

	_commandBuffer->GetDeletionQueue().push_function([=]() {
		vkDestroyImageView(_device, _drawImage->imageView, nullptr);
		vmaDestroyImage(_drawImageHandler->GetAllocator(), _drawImage->image, _drawImage->allocation);

		vkDestroyImageView(_device, _depthImage->imageView, nullptr);
		vmaDestroyImage(_drawImageHandler->GetAllocator(), _depthImage->image, _depthImage->allocation);
		});
}

void VulkanSwapchain::Destroy()
{
	vkDestroySwapchainKHR(_device, _swapchain, nullptr);

	for (int i = 0; i < _swapchainImageViews.size(); ++i)
	{
		vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
	}
}

void VulkanSwapchain::Resize(uint32_t width, uint32_t height, bool resizeRequested, GLFWwindow* window)
{
	vkDeviceWaitIdle(_device);

	Destroy();

	int w, h;
	glfwGetWindowSize(window, &w, &h);
	_windowExtent.width = w;
	_windowExtent.height = h;

	Create(_windowExtent.width, _windowExtent.height);

	resizeRequested = false;
}

void VulkanSwapchain::Present()
{

}


