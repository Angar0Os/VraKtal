#include "../src/core/gpu/vulkan/swapchain_impl.h"
#include "../src/core/gpu/vulkan/image_impl.h"
#include "../src/core/gpu_detail/converters.h"

#include <core/gpu/image.h>

#include <algorithm>
#include <stdexcept>

core::gpu::Swapchain::Impl::Impl(core::gpu::Swapchain& p, vk::raii::Device& dev,
	vk::raii::PhysicalDevice& physDev, const SwapchainCreateInfo& info)
	: parent(p), device(dev), physicalDevice(physDev), swapchain(nullptr)
{
	VkSurfaceKHR vkSurfaceHandle = reinterpret_cast<VkSurfaceKHR>(info.surface);
	vk::SurfaceKHR vkSurface(vkSurfaceHandle);

	vk::SurfaceCapabilitiesKHR capabilities = physicalDevice.getSurfaceCapabilitiesKHR(vkSurface);
	auto surfaceFormats = physicalDevice.getSurfaceFormatsKHR(vkSurface);
	auto presentModes = physicalDevice.getSurfacePresentModesKHR(vkSurface);

	vk::SurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(surfaceFormats, info.preferredFormat);
	vk::PresentModeKHR presentMode = ChoosePresentMode(presentModes, info.presentMode);
	extent = ChooseExtent(capabilities, info.width, info.height);

	format = surfaceFormat.format;

	uint32_t imageCount = std::max(info.minImageCount, capabilities.minImageCount);
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
	{
		imageCount = capabilities.maxImageCount;
	}

	vk::SwapchainCreateInfoKHR createInfo{};
	createInfo.surface = vkSurface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
	createInfo.imageSharingMode = vk::SharingMode::eExclusive;
	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	createInfo.presentMode = presentMode;
	createInfo.clipped = vk::True;
	createInfo.oldSwapchain = info.oldSwapchain ? reinterpret_cast<VkSwapchainKHR>(reinterpret_cast<uintptr_t>(info.oldSwapchain)) : VK_NULL_HANDLE;

	swapchain = vk::raii::SwapchainKHR(device, createInfo);

	images = GetImages();

	imageViews.clear();
	imageViews.reserve(images.size());

	for (const core::gpu::Image* image : images)
	{
		vk::ImageViewCreateInfo viewInfo{};
		vk::ImageSubresourceRange viewInfoSubResource{};
		viewInfo.image = image->GetImpl().image;
		viewInfo.viewType = vk::ImageViewType::e2D;
		viewInfo.format = format;

		viewInfoSubResource.aspectMask = vk::ImageAspectFlagBits::eColor;
		viewInfoSubResource.baseMipLevel = 0;
		viewInfoSubResource.levelCount = 1;
		viewInfoSubResource.baseArrayLayer = 0;
		viewInfoSubResource.layerCount = 1;

		viewInfo.subresourceRange = viewInfoSubResource;

		imageViews.emplace_back(device, viewInfo);
	}
}

core::gpu::Swapchain::Impl::~Impl() = default;

vk::SurfaceFormatKHR core::gpu::Swapchain::Impl::ChooseSurfaceFormat(
	const std::vector<vk::SurfaceFormatKHR>& availableFormats,
	TextureFormat preferredFormat)
{
	vk::Format vkPreferredFormat = core::gpu_detail::ToVulkan(preferredFormat);

	for (const auto& format : availableFormats)
	{
		if (format.format == vkPreferredFormat &&
			format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			return format;
		}
	}

	for (const auto& format : availableFormats)
	{
		if (format.format == vk::Format::eB8G8R8A8Srgb &&
			format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			return format;
		}
	}

	return availableFormats[0];
}

vk::PresentModeKHR core::gpu::Swapchain::Impl::ChoosePresentMode(
	const std::vector<vk::PresentModeKHR>& availableModes,
	PresentMode preferredMode)
{
	vk::PresentModeKHR vkPreferredMode = core::gpu_detail::ToVulkan(preferredMode);

	for (const auto& mode : availableModes)
	{
		if (mode == vkPreferredMode)
		{
			return mode;
		}
	}

	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D core::gpu::Swapchain::Impl::ChooseExtent(
	const vk::SurfaceCapabilitiesKHR& capabilities,
	uint32_t width, uint32_t height)
{
	if (capabilities.currentExtent.width != 0xFFFFFFFF)
	{
		return capabilities.currentExtent;
	}

	vk::Extent2D actualExtent = { width, height };

	actualExtent.width = std::clamp(actualExtent.width,
		capabilities.minImageExtent.width,
		capabilities.maxImageExtent.width);
	actualExtent.height = std::clamp(actualExtent.height,
		capabilities.minImageExtent.height,
		capabilities.maxImageExtent.height);

	return actualExtent;
}

vk::raii::SwapchainKHR& core::gpu::Swapchain::Impl::GetSwapchain()
{
	return swapchain;
}

const vk::raii::SwapchainKHR& core::gpu::Swapchain::Impl::GetSwapchain() const
{
	return swapchain;
}

uint32_t core::gpu::Swapchain::Impl::GetImageCount() const
{
	return static_cast<uint32_t>(images.size());
}

const core::gpu::Image* core::gpu::Swapchain::Impl::GetImage(uint32_t index) const
{
	if (index >= images.size())
	{
		throw std::out_of_range("Swapchain image index out of range");
	}

	return images[index];
}

core::TextureFormat core::gpu::Swapchain::Impl::GetFormat() const
{
	return core::gpu_detail::FromVulkan(format);
}

uint32_t core::gpu::Swapchain::Impl::GetWidth() const
{
	return extent.width;
}

uint32_t core::gpu::Swapchain::Impl::GetHeight() const
{
	return extent.height;
}

uint32_t core::gpu::Swapchain::Impl::AcquireNextImage(vk::Semaphore semaphore, uint64_t timeout)
{
	auto [result, imageIndex] = swapchain.acquireNextImage(timeout, semaphore, nullptr);

	if (result == vk::Result::eErrorOutOfDateKHR)
	{
		throw std::runtime_error("Swapchain out of date");
	}
	else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
	{
		throw std::runtime_error("Failed to acquire swapchain image");
	}

	return imageIndex;
}


std::vector<const core::gpu::Image*> core::gpu::Swapchain::Impl::GetImages() const
{
	std::vector<const core::gpu::Image*> result;
	result.reserve(GetImageCount());

	for(uint32_t i = 0; i < GetImageCount(); ++i)
	{
		result.push_back(GetImage(i));
	}

	return result;
}

core::gpu::Swapchain::Swapchain(void* device, void* physicalDevice, const SwapchainCreateInfo& info)
{
	auto& vkDevice = *static_cast<vk::raii::Device*>(device);
	auto& vkPhysicalDevice = *static_cast<vk::raii::PhysicalDevice*>(physicalDevice);

	m_impl = std::make_unique<Impl>(*this, vkDevice, vkPhysicalDevice, info);
}

core::gpu::Swapchain::~Swapchain() = default;

core::gpu::Swapchain::Swapchain(Swapchain&&) noexcept = default;
core::gpu::Swapchain& core::gpu::Swapchain::operator=(Swapchain&&) noexcept = default;

void* core::gpu::Swapchain::GetHandle() const
{
	return reinterpret_cast<void*>(static_cast<VkSwapchainKHR>(*m_impl->GetSwapchain()));
}

uint32_t core::gpu::Swapchain::GetImageCount() const
{
	return m_impl->GetImageCount();
}

const core::gpu::Image* core::gpu::Swapchain::GetImage(uint32_t index) const
{
	return m_impl->GetImage(index);
}

std::vector<const core::gpu::Image*> core::gpu::Swapchain::GetImages() const
{
	return m_impl->GetImages();
}

core::TextureFormat core::gpu::Swapchain::GetFormat() const
{
	return m_impl->GetFormat();
}

uint32_t core::gpu::Swapchain::GetWidth() const
{
	return m_impl->GetWidth();
}

uint32_t core::gpu::Swapchain::GetHeight() const
{
	return m_impl->GetHeight();
}

uint32_t core::gpu::Swapchain::AcquireNextImage(void* semaphore, uint64_t timeout)
{
	auto* vkSemaphore = static_cast<vk::Semaphore*>(semaphore);
	return m_impl->AcquireNextImage(*vkSemaphore, timeout);
}

core::gpu::Swapchain::Impl& core::gpu::Swapchain::GetImpl()
{
	return *m_impl;
}

const core::gpu::Swapchain::Impl& core::gpu::Swapchain::GetImpl() const
{
	return *m_impl;
}