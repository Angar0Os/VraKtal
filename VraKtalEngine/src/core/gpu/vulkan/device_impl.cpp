#define NOMINMAX // Disable Windows min/max macros which conflict with std::min/max
#define LAB_TASK_LEVEL 1

#include "../src/core/gpu/vulkan/device_impl.h"
#include "../src/core/gpu/vulkan/accelerationStructure_impl.h"
#include "../src/core/gpu/vulkan/commandPool_impl.h"
#include "../src/core/gpu/vulkan/image_impl.h"
#include "../src/core/gpu/vulkan/imguiContext_impl.h"

#include "../src/core/gpu_detail/converters.h"

#include <core/enum.h>

#include <graphics/resources/object/mesh.h>

#include <iostream>
#include <stdexcept>
#include <GLFW/glfw3.h>

#include <fstream>

using namespace core;
using namespace core::gpu;

#pragma comment(lib, "vulkan-1.lib")

// Validation layers used in debug
const std::vector<char const*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

std::vector<const char*> GetRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	if (enableValidationLayers)
	{
		extensions.push_back(vk::EXTDebugUtilsExtensionName);
	}

	return extensions;
}

::Device::Impl& ::Device::GetImpl() const
{
	return *m_impl;
}

CommandPool& ::Device::GetCommandPool() const
{
	return *m_impl->commandPool;
}

DescriptorPool& ::Device::GetDescriptorPool() const
{
	return *m_impl->descriptorPool;
}

::Device::Device(core::Window& window)
{
	m_impl = std::make_unique<Impl>(window, nullptr);
	m_impl->parent = this;
	m_impl->Initialize();
	m_imGuiContext = new ImguiContext(window, *this);
}

::Device::~Device()
{
	if (m_imGuiContext)
	{
		delete m_imGuiContext;
		m_imGuiContext = nullptr;
	}
}

::Device::Impl::Impl(core::Window& _window, const ::Device* _parent)
	: m_window(_window), parent(_parent)
{
}

void ::Device::Impl::Initialize()
{
	CreateInstance();
	SetupDebugMessenger();
	CreateSurface();
	PickPhysicalDevice();
	CreateLogicalDevice();

	CreateSwapchain();

	CreateDescriptorPool();
	CreateCommandPool();

	CreateSyncObjects();
}

::Device::Impl::~Impl()
{
	descriptorPool.reset();
}

void ::Device::Impl::CreateInstance()
{
	// Application info for the instance: not required but helpful for drivers		
	constexpr vk::ApplicationInfo appInfo
	{
		.pApplicationName = "Hello Triangle",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = vk::ApiVersion14
	};

	// If we requested validation layers, populate the vector
	std::vector<char const*> requiredLayers;
	if (enableValidationLayers)
	{
		requiredLayers.assign(validationLayers.begin(), validationLayers.end());
	}

	// Verify that the requested layers are available
	auto layerProperties = context.enumerateInstanceLayerProperties();
	if (std::ranges::any_of(requiredLayers, [&layerProperties](auto const& requiredLayer) {
		return std::ranges::none_of(layerProperties,
			[requiredLayer](auto const& layerProperty)
			{ return strcmp(layerProperty.layerName, requiredLayer) == 0; });
		}))
	{
		throw std::runtime_error("One or more required layers are not supported!");
	}

	// Get extensions required by GLFW and optionally debug utils
	auto requiredExtensions = GetRequiredExtensions();

	// Verify required extensions are available
	auto extensionProperties = context.enumerateInstanceExtensionProperties();
	for (auto const& requiredExtension : requiredExtensions)
	{
		if (std::ranges::none_of(extensionProperties,
			[requiredExtension](auto const& extensionProperty)
			{ return strcmp(extensionProperty.extensionName, requiredExtension) == 0; }))
		{
			throw std::runtime_error("Required extension not supported: " + std::string(requiredExtension));
		}
	}

	// Create instance
	vk::InstanceCreateInfo createInfo
	{
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
		.ppEnabledLayerNames = requiredLayers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
		.ppEnabledExtensionNames = requiredExtensions.data()
	};

	instance = vk::raii::Instance(context, createInfo);
}

// Note : This function must be above SetupDebugMessenger() because we did not declared it on .h file
static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
	vk::DebugUtilsMessageTypeFlagsEXT type,
	const vk::DebugUtilsMessengerCallbackDataEXT*
	pCallbackData, void*)
{
	std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
	return vk::False;
}

void ::Device::Impl::SetupDebugMessenger()
{
	if (!enableValidationLayers) return;

	vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

	vk::DebugUtilsMessageTypeFlagsEXT    messageTypeFlags(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
		vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
		vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

	vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT
	{
		.messageSeverity = severityFlags,
		.messageType = messageTypeFlags,
		.pfnUserCallback = &DebugCallback
	};

	debugMessenger = instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
}

void ::Device::Impl::CreateSurface()
{
	GLFWwindow* glfwWindow = m_window.GlfwHandle();
	if (!glfwWindow) {
		throw std::runtime_error("Invalid GLFW window handle!");
	}

	VkSurfaceKHR _surface;
	VkResult result = glfwCreateWindowSurface(*instance, glfwWindow, nullptr, &_surface);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface. Error code: " + std::to_string(result));
	}
	surface = vk::raii::SurfaceKHR(instance, _surface);
}

void ::Device::Impl::PickPhysicalDevice()
{
	std::vector<vk::raii::PhysicalDevice> devices = instance.enumeratePhysicalDevices();

	vk::raii::PhysicalDevice* bestDevice = nullptr;
	int bestScore = -1;

	for (auto& device : devices)
	{
		auto props = device.getProperties();
		int score = 0;

		auto queueFamilies = device.getQueueFamilyProperties();
		bool supportsGraphics = std::ranges::any_of(queueFamilies,
			[](auto const& qfp) { return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics); });

		auto availableDeviceExtensions = device.enumerateDeviceExtensionProperties();
		bool supportsAllRequiredExtensions = true;

		std::vector<const char*> requiredExtensions = {
			vk::KHRSwapchainExtensionName,
			vk::KHRSpirv14ExtensionName,
			vk::KHRSynchronization2ExtensionName,
			vk::KHRCreateRenderpass2ExtensionName
		};

		for (const auto& requiredExt : requiredExtensions)
		{
			bool found = std::ranges::any_of(availableDeviceExtensions,
				[requiredExt](auto const& availableExt)
				{ return strcmp(availableExt.extensionName, requiredExt) == 0; });

			if (!found)
			{
				supportsAllRequiredExtensions = false;
				break;
			}
		}

		if (!supportsAllRequiredExtensions) continue;

		auto basicFeatures = device.template getFeatures2
			<vk::PhysicalDeviceFeatures2,
			vk::PhysicalDeviceVulkan13Features,
			vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

		bool samplerAniso = basicFeatures.template get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy;
		bool dynRender = basicFeatures.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering;
		bool extDynState = basicFeatures.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

		std::vector<const char*> rtExtensions = {
			VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
			VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
			VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
			VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
			VK_KHR_RAY_QUERY_EXTENSION_NAME,
			VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
			VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME
		};

		bool supportsAllRTExtensions = true;
		for (const auto& rtExt : rtExtensions)
		{
			bool found = std::ranges::any_of(availableDeviceExtensions,
				[rtExt](auto const& availableExt)
				{ return strcmp(availableExt.extensionName, rtExt) == 0; });

			if (!found)
			{
				supportsAllRTExtensions = false;
				break;
			}
		}

		if (!supportsAllRTExtensions) continue;

		auto rtFeatures = device.template getFeatures2
			<vk::PhysicalDeviceFeatures2,
			vk::PhysicalDeviceBufferDeviceAddressFeatures,
			vk::PhysicalDeviceAccelerationStructureFeaturesKHR,
			vk::PhysicalDeviceRayTracingPipelineFeaturesKHR,
			vk::PhysicalDeviceRayQueryFeaturesKHR>();

		bool bufferAddr = rtFeatures.template get<vk::PhysicalDeviceBufferDeviceAddressFeatures>().bufferDeviceAddress;
		bool accelStruct = rtFeatures.template get<vk::PhysicalDeviceAccelerationStructureFeaturesKHR>().accelerationStructure;
		bool rtPipeline = rtFeatures.template get<vk::PhysicalDeviceRayTracingPipelineFeaturesKHR>().rayTracingPipeline;
		bool rayQuery = rtFeatures.template get<vk::PhysicalDeviceRayQueryFeaturesKHR>().rayQuery;

		if (props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
			score += 1000;
		else if (props.deviceType == vk::PhysicalDeviceType::eIntegratedGpu)
			score += 100;

		score += props.limits.maxImageDimension2D / 1000;

		if (score > bestScore)
		{
			bestScore = score;
			bestDevice = &device;
		}
	}

	if (bestDevice)
	{
		auto props = bestDevice->getProperties();
		physicalDevice = *bestDevice;
	}
	else
	{
		throw std::runtime_error("Failed to find a suitable GPU with raytracing support!");
	}
}

void ::Device::Impl::CreateLogicalDevice()
{
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

	for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); ++qfpIndex)
	{
		if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
			physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface))
		{
			queueIndex = qfpIndex;
			break;
		}
	}

	if (queueIndex == ~0)
	{
		throw std::runtime_error("Could not find a queue for graphics and present");
	}

	vk::PhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures{};
	rayQueryFeatures.rayQuery = VK_TRUE;

	vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rtPipelineFeatures{};
	rtPipelineFeatures.rayTracingPipeline = VK_TRUE;
	rtPipelineFeatures.pNext = &rayQueryFeatures;

	vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelFeatures{};
	accelFeatures.accelerationStructure = VK_TRUE;
	accelFeatures.pNext = &rtPipelineFeatures;

	vk::PhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures{};
	bufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;
	bufferDeviceAddressFeatures.pNext = &accelFeatures;

	vk::StructureChain
		<vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan11Features,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain = {
	   {.features = {.samplerAnisotropy = true} },
	   {.shaderDrawParameters = true},
	   {.synchronization2 = true, .dynamicRendering = true},
	   {.extendedDynamicState = true}
	};

	featureChain.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().pNext = &bufferDeviceAddressFeatures;

	float queuePriority = 1.0f;
	vk::DeviceQueueCreateInfo deviceQueueCreateInfo{
		.queueFamilyIndex = queueIndex,
		.queueCount = 1,
		.pQueuePriorities = &queuePriority
	};

	vk::DeviceCreateInfo deviceCreateInfo{
		.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &deviceQueueCreateInfo,
		.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtension.size()),
		.ppEnabledExtensionNames = requiredDeviceExtension.data()
	};

	device = vk::raii::Device(physicalDevice, deviceCreateInfo);
	graphicsQueue = vk::raii::Queue(device, queueIndex, 0);

	auto rtProps = physicalDevice.getProperties2
		<vk::PhysicalDeviceProperties2,
		vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>();

	const auto& rtPipelineProps = rtProps.get<vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>();
}

void ::Device::Impl::CreateDescriptorPool()
{
	descriptorPool = std::make_unique<DescriptorPool>(parent);
}

void ::Device::Impl::CreateCommandPool()
{
	CommandPoolCreateInfo poolInfo{
		.queueFamilyIndex = queueIndex,
		.flags = CommandPoolCreateFlags::ResetCommandBuffer
	};

	commandPool = std::make_unique<CommandPool>(parent, poolInfo);
}

vk::SurfaceFormatKHR Device::Impl::ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats, TextureFormat preferredFormat)
{
	vk::Format vkPreferredFormat = gpu_detail::ToVulkan(preferredFormat);

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

vk::PresentModeKHR Device::Impl::ChoosePresentMode(const std::vector<vk::PresentModeKHR>& availableModes, PresentMode preferredMode)
{
	vk::PresentModeKHR vkPreferredMode = gpu_detail::ToVulkan(preferredMode);

	for (const auto& mode : availableModes)
	{
		if (mode == vkPreferredMode)
		{
			return mode;
		}
	}

	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Device::Impl::ChooseExtent(const vk::SurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height)
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


void Device::Impl::CreateSwapchain()
{
	vk::SurfaceCapabilitiesKHR capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
	auto surfaceFormats = physicalDevice.getSurfaceFormatsKHR(surface);
	auto presentModes = physicalDevice.getSurfacePresentModesKHR(surface);

	vk::SurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(surfaceFormats, TextureFormat::RGBA8_SRGB);
	vk::PresentModeKHR presentMode = ChoosePresentMode(presentModes, PresentMode::Fifo);
	vk::Extent2D extent = ChooseExtent(capabilities, 0, 0);

	uint32_t imageCount = std::max(2u, capabilities.minImageCount);
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
	{
		imageCount = capabilities.maxImageCount;
	}

	vk::SwapchainCreateInfoKHR createInfo{
		.surface = surface,
		.minImageCount = imageCount,
		.imageFormat = surfaceFormat.format,
		.imageColorSpace = surfaceFormat.colorSpace,
		.imageExtent = extent,
		.imageArrayLayers = 1,
		.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,
		.imageSharingMode = vk::SharingMode::eExclusive,
		.preTransform = capabilities.currentTransform,
		.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		.presentMode = presentMode,
		.clipped = vk::True,
		.oldSwapchain = nullptr
	};

	swapchain = vk::raii::SwapchainKHR(device, createInfo);
	swapchainExtent = extent;

	std::vector<vk::Image> vkImages = swapchain.getImages();
	swapchainImageViews.clear();
	swapchainImageViews.reserve(vkImages.size());

	for (vk::Image image : vkImages)
	{
		vk::ImageViewCreateInfo viewInfo{
			.image = image,
			.viewType = vk::ImageViewType::e2D,
			.format = surfaceFormat.format,
			.subresourceRange = {
				.aspectMask = vk::ImageAspectFlagBits::eColor,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};
		swapchainImageViews.emplace_back(device, viewInfo);

		gpu::SPredefinedImageCreateInfo imageInfo{};
		imageInfo.image = image;
		imageInfo.extent = extent;
		imageInfo.aspectFlags = vk::ImageAspectFlagBits::eColor;
		imageInfo.format = surfaceFormat.format;

		swapchainImages.emplace_back(std::make_unique<Image>(parent, imageInfo));
	}

	swapchainImageFormat = surfaceFormat.format;
}

void ::Device::Impl::RecreateSwapchain()
{
	needsResize = false;
	int width = 0, height = 0;
	glfwGetFramebufferSize(m_window.GlfwHandle(), &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(m_window.GlfwHandle(), &width, &height);
		glfwWaitEvents();
	}

	device.waitIdle();

	swapchainImageViews.clear();
	swapchainImages.clear();

	vk::SwapchainKHR oldSwapchain = *swapchain;

	vk::SurfaceCapabilitiesKHR capabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
	auto surfaceFormats = physicalDevice.getSurfaceFormatsKHR(*surface);
	auto presentModes = physicalDevice.getSurfacePresentModesKHR(*surface);

	vk::SurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(surfaceFormats, TextureFormat::RGBA8_SRGB);
	vk::PresentModeKHR   presentMode = ChoosePresentMode(presentModes, PresentMode::Mailbox);
	vk::Extent2D         extent = ChooseExtent(capabilities, width, height);

	uint32_t imageCount = std::max(3u, capabilities.minImageCount);
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
		imageCount = capabilities.maxImageCount;

	vk::SwapchainCreateInfoKHR createInfo{
		.surface = *surface,
		.minImageCount = imageCount,
		.imageFormat = surfaceFormat.format,
		.imageColorSpace = surfaceFormat.colorSpace,
		.imageExtent = extent,
		.imageArrayLayers = 1,
		.imageUsage = vk::ImageUsageFlagBits::eColorAttachment
						  | vk::ImageUsageFlagBits::eTransferDst,
		.imageSharingMode = vk::SharingMode::eExclusive,
		.preTransform = capabilities.currentTransform,
		.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		.presentMode = presentMode,
		.clipped = vk::True,
		.oldSwapchain = oldSwapchain
	};

	swapchain = vk::raii::SwapchainKHR(device, createInfo);
	swapchainExtent = extent;
	swapchainImageFormat = surfaceFormat.format;

	std::vector<vk::Image> vkImages = swapchain.getImages();
	swapchainImageViews.reserve(vkImages.size());

	for (vk::Image image : vkImages)
	{
		vk::ImageViewCreateInfo viewInfo{
			.image = image,
			.viewType = vk::ImageViewType::e2D,
			.format = surfaceFormat.format,
			.subresourceRange = {
				.aspectMask = vk::ImageAspectFlagBits::eColor,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};
		swapchainImageViews.emplace_back(device, viewInfo);

		::SPredefinedImageCreateInfo imageInfo{};
		imageInfo.image = image;
		imageInfo.extent = extent;
		imageInfo.aspectFlags = vk::ImageAspectFlagBits::eColor;
		imageInfo.format = surfaceFormat.format;
		swapchainImages.emplace_back(std::make_unique<Image>(parent, imageInfo));
	}

	CreateSyncObjects();
}

void ::Device::Impl::CreateSyncObjects()
{
	frameSyncObjects.clear();
	tempCmdBufs.clear();

	vk::SemaphoreCreateInfo semInfo{};
	vk::FenceCreateInfo fenceInfo{ .flags = vk::FenceCreateFlagBits::eSignaled };

	frameSyncObjects.reserve(::Device::s_FRAMES_IN_FLIGHT);
	for (size_t i = 0; i < ::Device::s_FRAMES_IN_FLIGHT; ++i)
	{
		FrameSync frameSync{
			.inFlightFence = vk::raii::Fence(device, fenceInfo),
			.imageAvailable = vk::raii::Semaphore(device, semInfo),
			.renderFinished = vk::raii::Semaphore(device, semInfo)
		};

		frameSyncObjects.push_back(std::move(frameSync));
	}

	uint32_t swapchainImageCount = swapchain.getImages().size();
	tempCmdBufs.resize(::Device::s_FRAMES_IN_FLIGHT);
}

void Device::BeginFrame(uint32_t frameIndex)
{
	if (frameIndex >= m_impl->frameSyncObjects.size())
	{
		return;
	}

	auto& frameSync = m_impl->frameSyncObjects[frameIndex];

	m_impl->device.waitForFences(*frameSync.inFlightFence, VK_TRUE, UINT64_MAX);
	m_impl->device.resetFences(*frameSync.inFlightFence);

	if (frameIndex < m_impl->tempCmdBufs.size())
	{
		m_impl->tempCmdBufs[frameIndex].reset();
	}
}

void Device::Impl::TransitionImageForPresent(uint32_t frameIndex, uint32_t imageIndex)
{
	const ::Image* swapchainImage = swapchainImages[imageIndex].get();

	vk::CommandBufferAllocateInfo allocInfo{
		.commandPool = commandPool->GetImpl().pool,
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = 1
	};

	try
	{
		std::vector<vk::raii::CommandBuffer> cmdBufs = device.allocateCommandBuffers(allocInfo);
		if (cmdBufs.empty()) return;

		vk::raii::CommandBuffer cmdBuf = std::move(cmdBufs[0]);

		vk::CommandBufferBeginInfo beginInfo{
			.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
		};
		cmdBuf.begin(beginInfo);

		vk::ImageMemoryBarrier barrier{
			.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
			.dstAccessMask = vk::AccessFlagBits::eMemoryRead,
			.oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
			.newLayout = vk::ImageLayout::ePresentSrcKHR,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = swapchainImage->GetImpl().image,
			.subresourceRange = {
				.aspectMask = vk::ImageAspectFlagBits::eColor,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		cmdBuf.pipelineBarrier(
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::PipelineStageFlagBits::eBottomOfPipe,
			{},
			{},
			{},
			{ barrier }
		);

		cmdBuf.end();

		vk::CommandBuffer rawCmdBuf = *cmdBuf;

		vk::SubmitInfo submitInfo{
			.waitSemaphoreCount = 0,
			.pWaitSemaphores = nullptr,
			.pWaitDstStageMask = nullptr,
			.commandBufferCount = 1,
			.pCommandBuffers = &rawCmdBuf,
			.signalSemaphoreCount = 0,
			.pSignalSemaphores = nullptr
		};

		graphicsQueue.submit(submitInfo, nullptr);
		graphicsQueue.waitIdle();
	}
	catch (const vk::SystemError& e)
	{
		std::cerr << "Failed to transition image for present: " << e.what() << "\n";
	}
}

uint32_t Device::AcquireNextImage(uint32_t frameIndex)
{
	if (frameIndex >= m_impl->frameSyncObjects.size())
	{
		return UINT32_MAX;
	}

	auto& frameSync = m_impl->frameSyncObjects[frameIndex];

	vk::ResultValue<uint32_t> result = m_impl->device.acquireNextImage2KHR(
		vk::AcquireNextImageInfoKHR{
			.swapchain = m_impl->swapchain,
			.timeout = UINT64_MAX,
			.semaphore = *frameSync.imageAvailable,
			.fence = nullptr,
			.deviceMask = 1
		}
	);

	if (result.result == vk::Result::eErrorOutOfDateKHR)
	{
		return UINT32_MAX;
	}

	if (result.result != vk::Result::eSuccess &&
		result.result != vk::Result::eSuboptimalKHR)
	{
		return UINT32_MAX;
	}

	return result.value;
}

void Device::Present(uint32_t imageIndex, uint32_t frameIndex)
{
	if (frameIndex >= m_impl->frameSyncObjects.size()) return;

	auto& frameSync = m_impl->frameSyncObjects[frameIndex];
	vk::Semaphore presentWait = *frameSync.renderFinished;

	vk::PresentInfoKHR presentInfo{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &presentWait,
		.swapchainCount = 1,
		.pSwapchains = &*m_impl->swapchain,
		.pImageIndices = &imageIndex
	};

	try
	{
		vk::Result result = m_impl->graphicsQueue.presentKHR(presentInfo);
		if (result == vk::Result::eSuboptimalKHR)
		{
			m_impl->needsResize = true;
		}
	}
	catch (const vk::OutOfDateKHRError&)
	{
		m_impl->needsResize = true;
	}
}

void Device::Cleanup()
{
	m_impl->device.waitIdle();

	m_impl->frameSyncObjects.clear();
	m_impl->tempCmdBufs.clear();
}

const Image* Device::GetSwapchainImage(uint32_t imageIndex) const
{
	if (m_impl->swapchain == nullptr)
	{
		std::cerr << "ERROR: Swapchain is null!" << std::endl;
		return nullptr;
	}

	return m_impl->swapchainImages[imageIndex].get();
}

std::pair<uint32_t, uint32_t> Device::GetSwapchainExtent() const
{
	if (m_impl)
	{
		return { m_impl->swapchainExtent.width, m_impl->swapchainExtent.height };
	}
	return { 0, 0 };
}

void Device::WaitIdle()
{
	m_impl->device.waitIdle();
}

void Device::TransitionImageForPresent(uint32_t frameIndex, uint32_t imageIndex)
{
	if (m_impl) m_impl->TransitionImageForPresent(frameIndex, imageIndex);
}

bool core::gpu::Device::NeedsResize() const
{
	return m_impl->needsResize;
}

void core::gpu::Device::ClearResizeFlag()
{
	m_impl->needsResize = false;
}

void Device::RecreateSwapchain()
{
	if (m_impl) m_impl->RecreateSwapchain();
}

ImguiContext* Device::GetImGuiContext()
{
	return m_imGuiContext;
}