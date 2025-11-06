#define NOMINMAX // Disable Windows min/max macros which conflict with std::min/max

#include "../src/core/gpu/vulkan/device_impl.h"

#include <iostream>
#include <stdexcept>
#include <GLFW/glfw3.h>

#pragma comment(lib, "vulkan-1.lib")

// Validation layers used in debug
const std::vector<char const*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

// Note : We will maybe move this, but this is here to make uniform buffers work properly.
struct UniformBufferObject
{
	alignas(16) float model[16];
	alignas(16) float view[16];
	alignas(16) float projection[16];
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

core::gpu::Device::Impl& core::gpu::Device::GetImpl()
{
	return *m_impl;
}

core::gpu::Device::Device(core::Window& window)
{
	m_impl = std::make_unique<Impl>(window);
}

core::gpu::Device::~Device()
{

}

core::gpu::Device::Impl::Impl(const core::Window& window)
	: m_window(window)
{
	CreateInstance();
	SetupDebugMessenger();
	CreateSurface();
	PickPhysicalDevice();
	CreateLogicalDevice();
	CreateSwapchain();

	CreateDescriptorSetLayout();
	CreateDescriptorPool();
	AllocateDescriptorSets();
	CreateUniformBuffers();
	CreateCommandPool();
	CreateSamplers();
	CreateDefaultTextures();
	LoadMaterialTextures();
	CreateShadowMap();
	CreateDescriptorSets();
}

core::gpu::Device::Impl::~Impl()
{

}

void core::gpu::Device::Impl::CreateInstance()
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

void core::gpu::Device::Impl::SetupDebugMessenger()
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

void core::gpu::Device::Impl::CreateSurface()
{
	VkSurfaceKHR _surface;
	if (glfwCreateWindowSurface(*instance, m_window.GlfwHandle(), nullptr, &_surface) != 0)
	{
		throw std::runtime_error("Failed to create window surface.");
	}
	surface = vk::raii::SurfaceKHR(instance, _surface);
}

void core::gpu::Device::Impl::PickPhysicalDevice()
{
	std::vector<vk::raii::PhysicalDevice> devices = instance.enumeratePhysicalDevices();
	const auto                            devIter = std::ranges::find_if(
		devices,
		[&](auto const& device)
		{
			// Require Vulkan 1.3+ support for some features used
			bool supportsVulkan1_3 = device.getProperties().apiVersion >= VK_API_VERSION_1_3;

			// Choose a device with graphics queue support
			auto queueFamilies = device.getQueueFamilyProperties();
			bool supportsGraphics =
				std::ranges::any_of(queueFamilies, [](auto const& qfp) { return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics); });

			// Check device extension support
			auto availableDeviceExtensions = device.enumerateDeviceExtensionProperties();
			bool supportsAllRequiredExtensions =
				std::ranges::all_of(requiredDeviceExtension,
					[&availableDeviceExtensions](auto const& requiredDeviceExtension)
					{
						return std::ranges::any_of(availableDeviceExtensions,
							[requiredDeviceExtension](auto const& availableDeviceExtension)
							{ return strcmp(availableDeviceExtension.extensionName, requiredDeviceExtension) == 0; });
					});

			// Inspect features (using pNext chain to request feature structs)
			auto features = device.template getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
			bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy &&
				features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
				features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

			return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
		});
	if (devIter != devices.end())
	{
		physicalDevice = *devIter;
	}
	else
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

void core::gpu::Device::Impl::CreateLogicalDevice()
{
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

	// Find a queue family with graphics and present support
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
		throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
	}

	// Build a pNext chain to request features at device creation
	vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain =
	{
		{.features = {.samplerAnisotropy = true } },            // enable anisotropy
		{.synchronization2 = true, .dynamicRendering = true },  // enable synchronization2 and dynamic rendering
		{.extendedDynamicState = true }                         // enable extended dynamic state
	};

	float                     queuePriority = 0.0f;
	vk::DeviceQueueCreateInfo deviceQueueCreateInfo{ .queueFamilyIndex = queueIndex, .queueCount = 1, .pQueuePriorities = &queuePriority };
	vk::DeviceCreateInfo      deviceCreateInfo{ .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
												.queueCreateInfoCount = 1,
												.pQueueCreateInfos = &deviceQueueCreateInfo,
												.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtension.size()),
												.ppEnabledExtensionNames = requiredDeviceExtension.data() };

	device = vk::raii::Device(physicalDevice, deviceCreateInfo);
	graphicsQueue = vk::raii::Queue(device, queueIndex, 0);
}

// Choose a surface format that matches desired srgb format if available
vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			return availableFormat;
		}
	}
	
	// Fallback to the first supported format
	return availableFormats[0];
}

// Prefer mailbox present mode when available for low-latency
vk::PresentModeKHR ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == vk::PresentModeKHR::eMailbox)
		{
			return availablePresentMode;
		}
	}

	// FIFO is guaranteed to be available
	return vk::PresentModeKHR::eFifo;
}

// Compute swap extent (framebuffer size) choosing sensible defaults
vk::Extent2D core::gpu::Device::Impl::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != 0xFFFFFFFF)
	{
		return capabilities.currentExtent;
	}

	int width, height;
	glfwGetFramebufferSize(m_window.GlfwHandle(), &width, &height);

	return
	{
		std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
		std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
	};
}

// Choose min image count for the swapchain with a small guard for max supported
static uint32_t ChooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapabilities)
{
	auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
	if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount))
	{
		minImageCount = surfaceCapabilities.maxImageCount;
	}
	return minImageCount;
}

void core::gpu::Device::Impl::CreateSwapchain()
{
	auto surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
	swapChainSurfaceFormat = ChooseSwapSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(surface));
	swapChainExtent = ChooseSwapExtent(surfaceCapabilities);

	vk::SwapchainCreateInfoKHR swapChainCreateInfo
	{ 
		.surface = *surface,
		.minImageCount = ChooseSwapMinImageCount(surfaceCapabilities),
		.imageFormat = swapChainSurfaceFormat.format,
		.imageColorSpace = swapChainSurfaceFormat.colorSpace,
		.imageExtent = swapChainExtent,
		.imageArrayLayers = 1,
		.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
		.imageSharingMode = vk::SharingMode::eExclusive,
		.preTransform = surfaceCapabilities.currentTransform,
		.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		.presentMode = ChooseSwapPresentMode(physicalDevice.getSurfacePresentModesKHR(*surface)),
		.clipped = true 
	};

	swapChain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
	swapChainImages = swapChain.getImages();
}

// Note : We need to make this viable with our images probably ?
void core::gpu::Device::Impl::CreateImageViews()
{
	swapChainImageViews.clear();

	vk::ImageViewCreateInfo imageViewCreateInfo
	{
		.viewType = vk::ImageViewType::e2D,
		.format = swapChainSurfaceFormat.format,
		.subresourceRange =
		{
			vk::ImageAspectFlagBits::eColor,
			0,
			1,
			0,
			1
		}
	};

	for (auto image : swapChainImages)
	{
		imageViewCreateInfo.image = image;
		swapChainImageViews.emplace_back(device, imageViewCreateInfo);
	}
}

void core::gpu::Device::Impl::CreateDescriptorSetLayout()
{
	DescriptorSetLayoutCreateInfo layoutInfo;
	layoutInfo.bindings = 
	{
		{0, DescriptorType::UniformBuffer, 1, ShaderStage::Vertex | ShaderStage::Fragment},

		{1, DescriptorType::CombinedImageSampler, 1, ShaderStage::Fragment}, 
		{2, DescriptorType::CombinedImageSampler, 1, ShaderStage::Fragment}, 
		{3, DescriptorType::CombinedImageSampler, 1, ShaderStage::Fragment}, 
		{4, DescriptorType::CombinedImageSampler, 1, ShaderStage::Fragment}, 
		{5, DescriptorType::CombinedImageSampler, 1, ShaderStage::Fragment}, 
		{6, DescriptorType::CombinedImageSampler, 1, ShaderStage::Fragment}, 
		{7, DescriptorType::CombinedImageSampler, 1, ShaderStage::Fragment}  
	};

	descriptorSetLayout = std::make_unique<DescriptorSetLayout>(*device, layoutInfo);
}

void core::gpu::Device::Impl::CreateDescriptorPool()
{
	DescriptorPoolCreateInfo poolInfo;
	poolInfo.maxSets = MAX_FRAMES_IN_FLIGHT;
	poolInfo.poolSizes = 
	{
		{DescriptorType::UniformBuffer, MAX_FRAMES_IN_FLIGHT},
		{DescriptorType::CombinedImageSampler, MAX_FRAMES_IN_FLIGHT * 7}
	};
	poolInfo.allowFreeDescriptorSet = false;

	descriptorPool = std::make_unique<DescriptorPool>(*device, poolInfo);
}

void core::gpu::Device::Impl::AllocateDescriptorSets()
{
	std::vector<void*> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout->GetHandle());

	auto allocatedSets = descriptorPool->AllocateDescriptorSets(layouts, MAX_FRAMES_IN_FLIGHT);

	descriptorSets.clear();
	descriptorSets.reserve(allocatedSets.size());

	for (auto* setHandle : allocatedSets)
	{
		descriptorSets.push_back(setHandle);
	}
}

void core::gpu::Device::Impl::CreateUniformBuffers()
{
	uniformBuffers.clear();
	uniformBuffers.reserve(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		BufferCreateInfo bufferInfo{
			.size = sizeof(UniformBufferObject),
			.usage = BufferUsage::UniformBuffer,
			.memoryProperties = MemoryProperty::HostVisible | MemoryProperty::HostCoherent
		};
		uniformBuffers.push_back(std::make_unique<Buffer>(&device, &physicalDevice, bufferInfo));
	}
}

void core::gpu::Device::Impl::CreateSamplers()
{
	SamplerCreateInfo samplerInfo{
		.minFilter = Filter::Linear,
		.magFilter = Filter::Linear,
		.mipmapMode = SamplerMipmapMode::Linear,
		.addressModeU = SamplerAddressMode::Repeat,
		.addressModeV = SamplerAddressMode::Repeat,
		.addressModeW = SamplerAddressMode::Repeat,
		.mipLodBias = 0.0f,
		.enableAnisotropy = true,
		.maxAnisotropy = 8.0f,
		.enableCompare = false,
		.compareOp = CompareOp::Always,
		.minLod = 0.0f,
		.maxLod = 1000.0f
	};
	textureSampler = std::make_unique<Sampler>(&device, samplerInfo);

	SamplerCreateInfo shadowSamplerInfo{
		.minFilter = Filter::Linear,
		.magFilter = Filter::Linear,
		.addressModeU = SamplerAddressMode::ClampToBorder,
		.addressModeV = SamplerAddressMode::ClampToBorder,
		.addressModeW = SamplerAddressMode::ClampToBorder,
		.enableCompare = true,
		.compareOp = CompareOp::LessOrEqual
	};
	shadowSampler = std::make_unique<Sampler>(&device, shadowSamplerInfo);
}

void core::gpu::Device::Impl::CreateCommandPool()
{
	CommandPoolCreateInfo poolInfo{
		.queueFamilyIndex = queueIndex,
		.flags = CommandPoolCreateFlags::ResetCommandBuffer
	};

	commandPool = std::make_unique<CommandPool>(*device, poolInfo);
}


void core::gpu::Device::Impl::CreateDefaultTextures()
{
	defaultWhiteTexture = std::make_unique<Texture>(*device, *physicalDevice, *graphicsQueue, commandPool->GetHandle(), 1.0f, 1.0f, 1.0f, 1.0f);
	defaultBlackTexture = std::make_unique<Texture>(*device, *physicalDevice, *graphicsQueue, commandPool->GetHandle(), 0.0f, 0.0f, 0.0f, 1.0f);
	defaultNormalTexture = std::make_unique<Texture>(*device, *physicalDevice, *graphicsQueue, commandPool->GetHandle(), 0.5f, 0.5f, 1.0f, 1.0f);
}

void core::gpu::Device::Impl::LoadMaterialTextures()
{
	albedoTexture = std::make_unique<Texture>(*device, *physicalDevice, *graphicsQueue, commandPool->GetHandle(), 1.0f, 1.0f, 1.0f, 1.0f);
	albedoTexture->LoadTextureIfExists("assets/textures/albedo.png");

	normalTexture = std::make_unique<Texture>(*device, *physicalDevice, *graphicsQueue, commandPool->GetHandle(), 0.5f, 0.5f, 1.0f, 1.0f);
	normalTexture->LoadTextureIfExists("assets/textures/normal.png");

	metallicTexture = std::make_unique<Texture>(*device, *physicalDevice, *graphicsQueue, commandPool->GetHandle(), 1.0f, 1.0f, 1.0f, 1.0f);
	metallicTexture->LoadTextureIfExists("assets/textures/metallic.png");

	roughnessTexture = std::make_unique<Texture>(*device, *physicalDevice, *graphicsQueue, commandPool->GetHandle(), 1.0f, 1.0f, 1.0f, 1.0f);
	roughnessTexture->LoadTextureIfExists("assets/textures/roughness.png");

	aoTexture = std::make_unique<Texture>(*device, *physicalDevice, *graphicsQueue, commandPool->GetHandle(), 1.0f, 1.0f, 1.0f, 1.0f);
	aoTexture->LoadTextureIfExists("assets/textures/ao.png");

	emissiveTexture = std::make_unique<Texture>(*device, *physicalDevice, *graphicsQueue, commandPool->GetHandle(), 0.0f, 0.0f, 0.0f, 1.0f);
	emissiveTexture->LoadTextureIfExists("assets/textures/emissive.png");
}

void core::gpu::Device::Impl::CreateShadowMap()
{
	ImageCreateInfo shadowMapInfo{
		.width = 2048,
		.height = 2048,
		.mipLevels = 1,
		.format = TextureFormat::Depth32F,
		.tiling = ImageTiling::Optimal,
		.usage = ImageUsage::DepthStencilAttachment | ImageUsage::Sampled,
		.memoryProperties = MemoryProperty::DeviceLocal,
		.samples = SampleCount::e1
	};
	shadowMapImage = std::make_unique<Image>(&device, &physicalDevice, shadowMapInfo);

	ImageViewCreateInfo shadowViewInfo{
		.format = TextureFormat::Depth32F,
		.isDepth = true
	};
	shadowMapImage->CreateView(shadowViewInfo);
}

void core::gpu::Device::Impl::CreateDescriptorSets()
{
	std::vector<void*> descriptorSetHandles;
	descriptorSetHandles.reserve(descriptorSets.size());
	for (auto& set : descriptorSets)
	{
		descriptorSetHandles.push_back(&set);
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		DescriptorSet(&device, descriptorSetHandles, i)
			.BindBuffer(*uniformBuffers[i], 0, sizeof(UniformBufferObject))
			.BindImage(*textureSampler, albedoTexture.get(), *defaultWhiteTexture)
			.BindImage(*textureSampler, normalTexture.get(), *defaultNormalTexture)
			.BindImage(*textureSampler, metallicTexture.get(), *defaultWhiteTexture)
			.BindImage(*textureSampler, roughnessTexture.get(), *defaultWhiteTexture)
			.BindImage(*textureSampler, aoTexture.get(), *defaultWhiteTexture)
			.BindImage(*textureSampler, emissiveTexture.get(), *defaultBlackTexture)
			.BindImage(*shadowSampler, nullptr, *defaultWhiteTexture, ImageLayout::DepthStencilAttachment)
			.Update();
	}
}