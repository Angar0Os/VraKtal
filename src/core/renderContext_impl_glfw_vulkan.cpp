#include "renderContext_impl_glfw_vulkan.h"

#include <GLFW/glfw3.h>	

#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "vulkan-1.lib")

using namespace core::rhi;

RenderContext::RenderContext(const RenderContextDescriptor& descriptor)
	: m_Internal(new Internal)
{
	if (!glfwInit())
	{
		throw std::runtime_error("Failed to initialize GLFW");
		glfwTerminate();
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, descriptor.resizeable ? GLFW_TRUE : GLFW_FALSE);
	m_Internal->window = glfwCreateWindow(descriptor.windowSize.x, descriptor.windowSize.y, descriptor.windowTitle, nullptr, nullptr);

	if (!m_Internal->window)
	{
		throw std::runtime_error("Failed to create GLFW window");
		glfwTerminate();
	}

	vkb::InstanceBuilder builder;

	auto inst_ret = builder.set_app_name(descriptor.windowTitle)
		.request_validation_layers(m_Internal->useValidationLayers)
		.use_default_debug_messenger()
		.require_api_version(1, 3, 0)
		.build();

	vkb::Instance vkb_inst = inst_ret.value();

	m_Internal->instance.instance = vkb_inst.instance;
	m_Internal->debugMessenger = vkb_inst.debug_messenger;

	glfwCreateWindowSurface(m_Internal->instance, m_Internal->window, nullptr, &m_Internal->surface);

	VkPhysicalDeviceVulkan13Features features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
	features.dynamicRendering = true;
	features.synchronization2 = true;

	vkb::PhysicalDeviceSelector selector{ m_Internal->instance };
	vkb::PhysicalDevice physicalDevice = selector
		.set_minimum_version(1, 3)
		.set_required_features_13(features)
		.set_surface(m_Internal->surface)
		.select()
		.value();

	vkb::DeviceBuilder deviceBuilder{ physicalDevice };
	vkb::Device vkbDevice = deviceBuilder.build().value();

	m_Internal->device = vkbDevice.device;
	m_Internal->chosenGPU = physicalDevice.physical_device;

	m_Internal->graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
	m_Internal->graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = m_Internal->chosenGPU;
	allocatorInfo.device = m_Internal->device;
	allocatorInfo.instance = m_Internal->instance;
	allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	//vmaCreateAllocator(&allocatorInfo, &m_Internal->allocator);
}

void RenderContext::Internal::CreateSwapchain(uint32_t width, uint32_t height)
{
	vkb::SwapchainBuilder swapchainBuilder{ chosenGPU, device, surface };

	swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

	vkb::Swapchain vkbSwapchain = swapchainBuilder
		.set_desired_format(VkSurfaceFormatKHR{ .format = swapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.set_desired_extent(width, height)
		.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		.build()
		.value();

	swapchainExtent = vkbSwapchain.extent;
	swapchain = vkbSwapchain.swapchain;
	swapchainImages = vkbSwapchain.get_images().value();
	swapchainImageViews = vkbSwapchain.get_image_views().value();
}

RenderContext::~RenderContext()
{
	
}
