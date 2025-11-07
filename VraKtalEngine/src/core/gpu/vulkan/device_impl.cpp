#define NOMINMAX // Disable Windows min/max macros which conflict with std::min/max

#include "../src/core/gpu/vulkan/device_impl.h"

#include <iostream>
#include <stdexcept>
#include <GLFW/glfw3.h>

#include <core/enum.h>

#include <fstream>

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
	CreateShadowDescriptorSetLayout();

	CreateDescriptorPool();
	AllocateDescriptorSets();

	CreateUniformBuffers();
	CreateCommandPool();
	CreateSamplers();

	CreateDefaultTextures();
	LoadMaterialTextures();
	CreateShadowMap();

	CreateGraphicsPipeline();
	CreateShadowPipeline();

	CreateDescriptorSets();
	CreateShadowDescriptorSets();
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

void core::gpu::Device::Impl::CreateDescriptorSetLayout()
{
	DescriptorSetLayoutCreateInfo layoutInfo;
	layoutInfo.bindings = 
	{
		{0, DescriptorType::UniformBuffer, 1, core::ShaderStage::Vertex | core::ShaderStage::Fragment},

		{1, DescriptorType::CombinedImageSampler, 1, core::ShaderStage::Fragment},
		{2, DescriptorType::CombinedImageSampler, 1, core::ShaderStage::Fragment},
		{3, DescriptorType::CombinedImageSampler, 1, core::ShaderStage::Fragment},
		{4, DescriptorType::CombinedImageSampler, 1, core::ShaderStage::Fragment},
		{5, DescriptorType::CombinedImageSampler, 1, core::ShaderStage::Fragment},
		{6, DescriptorType::CombinedImageSampler, 1, core::ShaderStage::Fragment},
		{7, DescriptorType::CombinedImageSampler, 1, core::ShaderStage::Fragment}
	};

	descriptorSetLayout = std::make_unique<DescriptorSetLayout>(*device, layoutInfo);
}

void core::gpu::Device::Impl::CreateDescriptorPool()
{
	DescriptorPoolCreateInfo poolInfo;
	poolInfo.maxSets = MAX_FRAMES_IN_FLIGHT * 2; 
	poolInfo.poolSizes =
	{
		{DescriptorType::UniformBuffer, MAX_FRAMES_IN_FLIGHT * 2},
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

	std::vector<void*> shadowLayouts(MAX_FRAMES_IN_FLIGHT, shadowDescriptorSetLayout->GetHandle());
	auto allocatedShadowSets = descriptorPool->AllocateDescriptorSets(shadowLayouts, MAX_FRAMES_IN_FLIGHT);

	shadowDescriptorSets.clear();
	shadowDescriptorSets.reserve(allocatedShadowSets.size());
	for (auto* setHandle : allocatedShadowSets)
	{
		shadowDescriptorSets.push_back(setHandle);
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

void core::gpu::Device::Impl::CreateSwapchain()
{
	int width, height;
	glfwGetFramebufferSize(m_window.GlfwHandle(), &width, &height);

	SwapchainCreateInfo swapchainInfo{
		.surface = &surface,
		.width = static_cast<uint32_t>(width),
		.height = static_cast<uint32_t>(height),
		.preferredFormat = TextureFormat::RGBA8_SRGB,
		.presentMode = PresentMode::Mailbox,
		.minImageCount = 3,
		.oldSwapchain = nullptr
	};

	swapchain = std::make_unique<Swapchain>(&device, &physicalDevice, swapchainInfo);
}

void core::gpu::Device::Impl::CreateGraphicsPipeline()
{
	auto shaderCode = ReadFile("../bin/assets/shaders/slang.spv");

	VertexInputBinding vertexBinding{
		.binding = 0,
		.stride = sizeof(Vertex),  
		.inputRate = VertexInputRate::Vertex
	};

	std::vector<VertexInputAttribute> vertexAttributes = {
		{0, 0, TextureFormat::RGB32_Float, offsetof(Vertex, pos)},
		{1, 0, TextureFormat::RGB32_Float, offsetof(Vertex, color)},
		{2, 0, TextureFormat::RG32_Float, offsetof(Vertex, texCoord)},
		{3, 0, TextureFormat::RGB32_Float, offsetof(Vertex, normal)}
	};

	std::vector<ShaderStage> shaderStages = {
		{ShaderStageFlags::Vertex, shaderCode, "vertMain"},
		{ShaderStageFlags::Fragment, shaderCode, "fragMain"}
	};
	
	PipelineCreateInfo pipelineInfo{
		.shaderStages = shaderStages,
		.vertexBindings = {vertexBinding},
		.vertexAttributes = vertexAttributes,
		.topology = PrimitiveTopology::TriangleList,
		.polygonMode = PolygonMode::Fill,
		.cullMode = CullMode::Back,
		.frontFace = FrontFace::CounterClockwise,
		.depthTestEnable = true,
		.depthWriteEnable = true,
		.depthCompareOp = CompareOp::Less,
		.blendEnable = false,
		.samples = SampleCount::e4,  
		.colorAttachmentFormats = {swapchain->GetFormat()},
		.depthAttachmentFormat = TextureFormat::Depth32F,
		.descriptorSetLayouts = {descriptorSetLayout.get()},
		.dynamicStates = {DynamicState::Viewport, DynamicState::Scissor}
	};

	graphicsPipeline = std::make_unique<Pipeline>(&device, pipelineInfo);
}

void core::gpu::Device::Impl::CreateShadowPipeline()
{
	auto shaderCode = ReadFile("../bin/assets/shaders/slang.spv");

	VertexInputBinding vertexBinding{
		.binding = 0,
		.stride = sizeof(Vertex),
		.inputRate = VertexInputRate::Vertex
	};

	std::vector<VertexInputAttribute> vertexAttributes = {
		{0, 0, TextureFormat::RGB32_Float, offsetof(Vertex, pos)}
	};

	std::vector<ShaderStage> shaderStages = {
		{ShaderStageFlags::Vertex, shaderCode, "shadowMain"}
	};

	PipelineCreateInfo shadowInfo{
		.shaderStages = shaderStages,
		.vertexBindings = {vertexBinding},
		.vertexAttributes = vertexAttributes,
		.topology = PrimitiveTopology::TriangleList,
		.polygonMode = PolygonMode::Fill,
		.cullMode = CullMode::Back,
		.frontFace = FrontFace::CounterClockwise,
		.depthTestEnable = true,
		.depthWriteEnable = true,
		.depthCompareOp = CompareOp::LessOrEqual,
		.blendEnable = false,
		.samples = SampleCount::e1,
		.colorAttachmentFormats = {}, 
		.depthAttachmentFormat = TextureFormat::Depth32F,
		.descriptorSetLayouts = {shadowDescriptorSetLayout.get()},
		.dynamicStates = {
			DynamicState::Viewport,
			DynamicState::Scissor,
			DynamicState::DepthBias
		}
	};

	shadowPipeline = std::make_unique<Pipeline>(&device, shadowInfo);
}

std::vector<char> core::gpu::Device::Impl::ReadFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open shader file: " + filename);
	}

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}

void core::gpu::Device::Impl::RecreateSwapchain()
{
	device.waitIdle();

	int width, height;
	glfwGetFramebufferSize(m_window.GlfwHandle(), &width, &height);

	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(m_window.GlfwHandle(), &width, &height);
		glfwWaitEvents();
	}

	void* oldSwapchain = swapchain->GetHandle();

	SwapchainCreateInfo swapchainInfo{
		.surface = &surface,
		.width = static_cast<uint32_t>(width),
		.height = static_cast<uint32_t>(height),
		.preferredFormat = TextureFormat::RGBA8_SRGB,
		.presentMode = PresentMode::Mailbox,
		.minImageCount = 3,
		.oldSwapchain = oldSwapchain
	};

	swapchain = std::make_unique<Swapchain>(&device, &physicalDevice, swapchainInfo);
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

void core::gpu::Device::Impl::CreateShadowDescriptorSetLayout()
{
	DescriptorSetLayoutCreateInfo shadowLayoutInfo;
	shadowLayoutInfo.bindings =
	{
		{0, DescriptorType::UniformBuffer, 1, core::ShaderStage::Vertex}
	};

	shadowDescriptorSetLayout = std::make_unique<DescriptorSetLayout>(*device, shadowLayoutInfo);
}

void core::gpu::Device::Impl::CreateShadowDescriptorSets()
{
	std::vector<void*> descriptorSetHandles;
	descriptorSetHandles.reserve(shadowDescriptorSets.size());
	for (auto& set : shadowDescriptorSets)
	{
		descriptorSetHandles.push_back(&set);
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		DescriptorSet(&device, descriptorSetHandles, i)
			.BindBuffer(*uniformBuffers[i], 0, sizeof(UniformBufferObject))
			.Update();
	}
}