#define NOMINMAX // Disable Windows min/max macros which conflict with std::min/max

#include "../src/core/gpu/vulkan/device_impl.h"

#include <core/window.h>
#include <core/gpu/descriptorSet.h>
#include <core/gpu/buffer.h>
#include <core/gpu/sampler.h>
#include <core/gpu/image.h>
#include <core/gpu/commandBuffer.h>
#include <core/gpu/texture.h>

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

void core::gpu::Device::Impl::CreateDescriptorSets()
{
	vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
	vk::raii::DescriptorPool descriptorPool = nullptr;
	std::vector<vk::raii::DescriptorSet> descriptorSets;

	std::vector<vk::DescriptorSetLayoutBinding> bindings
	{
		vk::DescriptorSetLayoutBinding{
			.binding = 0,
			.descriptorType = vk::DescriptorType::eUniformBuffer,
			.descriptorCount = 1,
			.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
			.pImmutableSamplers = nullptr
		},
		vk::DescriptorSetLayoutBinding{
			.binding = 1,
			.descriptorType = vk::DescriptorType::eCombinedImageSampler,
			.descriptorCount = 1,
			.stageFlags = vk::ShaderStageFlagBits::eFragment,
			.pImmutableSamplers = nullptr
		},
		vk::DescriptorSetLayoutBinding{
			.binding = 2,
			.descriptorType = vk::DescriptorType::eCombinedImageSampler,
			.descriptorCount = 1,
			.stageFlags = vk::ShaderStageFlagBits::eFragment,
			.pImmutableSamplers = nullptr
		},
		vk::DescriptorSetLayoutBinding{
			.binding = 3,
			.descriptorType = vk::DescriptorType::eCombinedImageSampler,
			.descriptorCount = 1,
			.stageFlags = vk::ShaderStageFlagBits::eFragment,
			.pImmutableSamplers = nullptr
		},
		vk::DescriptorSetLayoutBinding{
			.binding = 4,
			.descriptorType = vk::DescriptorType::eCombinedImageSampler,
			.descriptorCount = 1,
			.stageFlags = vk::ShaderStageFlagBits::eFragment,
			.pImmutableSamplers = nullptr
		},
		vk::DescriptorSetLayoutBinding{
			.binding = 5,
			.descriptorType = vk::DescriptorType::eCombinedImageSampler,
			.descriptorCount = 1,
			.stageFlags = vk::ShaderStageFlagBits::eFragment,
			.pImmutableSamplers = nullptr
		},
		vk::DescriptorSetLayoutBinding{
			.binding = 6,
			.descriptorType = vk::DescriptorType::eCombinedImageSampler,
			.descriptorCount = 1,
			.stageFlags = vk::ShaderStageFlagBits::eFragment,
			.pImmutableSamplers = nullptr
		},
		vk::DescriptorSetLayoutBinding{
			.binding = 7,
			.descriptorType = vk::DescriptorType::eCombinedImageSampler,
			.descriptorCount = 1,
			.stageFlags = vk::ShaderStageFlagBits::eFragment,
			.pImmutableSamplers = nullptr
		}
	};

	vk::DescriptorSetLayoutCreateInfo layoutInfo{
		.bindingCount = static_cast<uint32_t>(bindings.size()),
		.pBindings = bindings.data()
	};

	descriptorSetLayout = vk::raii::DescriptorSetLayout(device, layoutInfo);

	std::vector<vk::DescriptorPoolSize> poolSizes{
		vk::DescriptorPoolSize{
			.type = vk::DescriptorType::eUniformBuffer,
			.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)
		},
		vk::DescriptorPoolSize{
			.type = vk::DescriptorType::eCombinedImageSampler,
			.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * 7)
		}
	};

	vk::DescriptorPoolCreateInfo poolInfo{
		.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
		.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
		.pPoolSizes = poolSizes.data()
	};

	descriptorPool = vk::raii::DescriptorPool(device, poolInfo);

	std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout);

	vk::DescriptorSetAllocateInfo allocInfo{
		.descriptorPool = *descriptorPool,
		.descriptorSetCount = static_cast<uint32_t>(layouts.size()),
		.pSetLayouts = layouts.data()
	};

	descriptorSets = device.allocateDescriptorSets(allocInfo);

	std::vector<std::unique_ptr<Buffer>> uniformBuffers;
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
	Sampler textureSampler(&device, samplerInfo);

	SamplerCreateInfo shadowSamplerInfo{
		.minFilter = Filter::Linear,
		.magFilter = Filter::Linear,
		.addressModeU = SamplerAddressMode::ClampToBorder,
		.addressModeV = SamplerAddressMode::ClampToBorder,
		.addressModeW = SamplerAddressMode::ClampToBorder,
		.enableCompare = true,
		.compareOp = CompareOp::LessOrEqual
	};
	Sampler shadowSampler(&device, shadowSamplerInfo);

	Texture defaultWhiteTexture(&device, &physicalDevice, &graphicsQueue, &commandPool, 1.0f, 1.0f, 1.0f, 1.0f);
	Texture defaultBlackTexture(&device, &physicalDevice, &graphicsQueue, &commandPool, 0.0f, 0.0f, 0.0f, 1.0f);
	Texture defaultNormalTexture(&device, &physicalDevice, &graphicsQueue, &commandPool, 0.5f, 0.5f, 1.0f, 1.0f);

	Texture* albedoTexture = nullptr;
	albedoTexture->LoadTextureIfExists("assets/textures/albedo.png");
	Texture* normalTexture = nullptr;
	normalTexture->LoadTextureIfExists("assets/textures/normal.png");
	Texture* metallicTexture = nullptr; 
	metallicTexture->LoadTextureIfExists("assets/textures/metallic.png");
	Texture* roughnessTexture = nullptr;
	roughnessTexture->LoadTextureIfExists("assets/textures/roughness.png");
	Texture* aoTexture = nullptr;
	aoTexture->LoadTextureIfExists("assets/textures/ao.png");
	Texture* emissiveTexture = nullptr;
	emissiveTexture->LoadTextureIfExists("assets/textures/emissive.png");

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
	Image shadowMapImage(&device, &physicalDevice, shadowMapInfo);

	ImageViewCreateInfo shadowViewInfo{
		.format = TextureFormat::Depth32F,
		.isDepth = true
	};
	shadowMapImage.CreateView(shadowViewInfo);

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
			.BindImage(textureSampler, albedoTexture, defaultWhiteTexture)
			.BindImage(textureSampler, normalTexture, defaultNormalTexture)
			.BindImage(textureSampler, metallicTexture, defaultWhiteTexture)
			.BindImage(textureSampler, roughnessTexture, defaultWhiteTexture)
			.BindImage(textureSampler, aoTexture, defaultWhiteTexture)
			.BindImage(textureSampler, emissiveTexture, defaultBlackTexture)
			.BindImage(shadowSampler, nullptr, defaultWhiteTexture, ImageLayout::DepthStencilAttachment)
			.Update();
	}
}