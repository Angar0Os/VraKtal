#define NOMINMAX // Disable Windows min/max macros which conflict with std::min/max

#include "../src/core/gpu/vulkan/device_impl.h"

#include <iostream>
#include <stdexcept>
#include <GLFW/glfw3.h>

#include <core/enum.h>

#include <graphics/resources/mesh.h>

#include <fstream>

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

core::gpu::Device::Impl::Impl(core::Window& window)
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
	CreateColorImage();

	CreateGraphicsPipeline();
	CreateShadowPipeline();

	CreateDescriptorSets();
	CreateShadowDescriptorSets();

	CreateSyncObjects();
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

	descriptorSetLayout = std::make_unique<DescriptorSetLayout>(&device, layoutInfo);
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
	poolInfo.allowFreeDescriptorSet = true;

	descriptorPool = std::make_unique<DescriptorPool>(&device, poolInfo);
}

void core::gpu::Device::Impl::AllocateDescriptorSets()
{
	std::vector<DescriptorSetLayout*> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout.get());
	auto allocatedSets = descriptorPool->AllocateDescriptorSets(layouts, MAX_FRAMES_IN_FLIGHT);

	descriptorSets.clear();
	descriptorSets.reserve(allocatedSets.size());
	for (auto* setHandle : allocatedSets)
	{
		descriptorSets.push_back(static_cast<vk::raii::DescriptorSet*>(setHandle));
	}

	std::vector<DescriptorSetLayout*> shadowLayouts(MAX_FRAMES_IN_FLIGHT, shadowDescriptorSetLayout.get());
	auto allocatedShadowSets = descriptorPool->AllocateDescriptorSets(shadowLayouts, MAX_FRAMES_IN_FLIGHT);

	shadowDescriptorSets.clear();
	shadowDescriptorSets.reserve(allocatedShadowSets.size());
	for (auto* setHandle : allocatedShadowSets)
	{
		shadowDescriptorSets.push_back(static_cast<vk::raii::DescriptorSet*>(setHandle));
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

	commandPool = std::make_unique<CommandPool>(&device, poolInfo);
}


void core::gpu::Device::Impl::CreateDefaultTextures()
{
	defaultWhiteTexture = std::make_unique<Texture>(&device, &physicalDevice, &graphicsQueue, commandPool->GetHandle(), 1.0f, 1.0f, 1.0f, 1.0f);
	defaultBlackTexture = std::make_unique<Texture>(&device, &physicalDevice, &graphicsQueue, commandPool->GetHandle(), 0.0f, 0.0f, 0.0f, 1.0f);
	defaultNormalTexture = std::make_unique<Texture>(&device, &physicalDevice, &graphicsQueue, commandPool->GetHandle(), 0.5f, 0.5f, 1.0f, 1.0f);
}

void core::gpu::Device::Impl::LoadMaterialTextures()
{
	albedoTexture = std::make_unique<Texture>(&device, &physicalDevice, &graphicsQueue,
		commandPool->GetHandle(), 1.0f, 1.0f, 1.0f, 1.0f);

	normalTexture = std::make_unique<Texture>(&device, &physicalDevice, &graphicsQueue,
		commandPool->GetHandle(), 0.5f, 0.5f, 1.0f, 1.0f);

	metallicTexture = std::make_unique<Texture>(&device, &physicalDevice, &graphicsQueue,
		commandPool->GetHandle(), 1.0f, 1.0f, 1.0f, 1.0f);

	roughnessTexture = std::make_unique<Texture>(&device, &physicalDevice, &graphicsQueue,
		commandPool->GetHandle(), 1.0f, 1.0f, 1.0f, 1.0f);

	aoTexture = std::make_unique<Texture>(&device, &physicalDevice, &graphicsQueue,
		commandPool->GetHandle(), 1.0f, 1.0f, 1.0f, 1.0f);

	emissiveTexture = std::make_unique<Texture>(&device, &physicalDevice, &graphicsQueue,
		commandPool->GetHandle(), 0.0f, 0.0f, 0.0f, 1.0f);

	bool loaded = albedoTexture->LoadTextureIfExists("../bin/assets/textures/viking_room.png");
	if (loaded)
	{
		std::cout << "Viking room texture loaded successfully!" << std::endl;
	}
	else
	{
		std::cerr << "Failed to load viking room texture, using default white" << std::endl;
	}

	normalTexture->LoadTextureIfExists("assets/textures/normal.png");
	metallicTexture->LoadTextureIfExists("assets/textures/metallic.png");
	roughnessTexture->LoadTextureIfExists("assets/textures/roughness.png");
	aoTexture->LoadTextureIfExists("assets/textures/ao.png");
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
		.samples = SampleCount::e4
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
		.surface = reinterpret_cast<void*>(static_cast<VkSurfaceKHR>(*surface)),
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
		.stride = sizeof(graphics::resources::Vertex),
		.inputRate = VertexInputRate::Vertex
	};

	std::vector<VertexInputAttribute> vertexAttributes = {
		{0, 0, TextureFormat::RGB32_Float, offsetof(graphics::resources::Vertex, position)},
		{2, 0, TextureFormat::RG32_Float, offsetof(graphics::resources::Vertex, uv)},
		{3, 0, TextureFormat::RGB32_Float, offsetof(graphics::resources::Vertex, normal)}
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
		.stride = sizeof(graphics::resources::Vertex),
		.inputRate = VertexInputRate::Vertex
	};

	std::vector<VertexInputAttribute> vertexAttributes = {
		{0, 0, TextureFormat::RGB32_Float, offsetof(graphics::resources::Vertex, position)}
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
	std::cout << "offsetof viewPos = " << offsetof(UniformBufferObject, viewPos) << std::endl;
	std::cout << "offsetof lights = " << offsetof(UniformBufferObject, lights) << std::endl;
	std::cout << "sizeof(LightData) = " << sizeof(UniformBufferObject::LightData) << std::endl;
	std::cout << "sizeof(UniformBufferObject) = " << sizeof(UniformBufferObject) << std::endl;
	std::cout << "offsetof useAlbedoMap = " << offsetof(UniformBufferObject, useAlbedoMap) << std::endl;
	std::cout << "offsetof numLights = " << offsetof(UniformBufferObject, numLights) << std::endl;
	std::cout << "offsetof albedo = " << offsetof(UniformBufferObject, albedo) << std::endl;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		DescriptorSet(&device, &descriptorSets, i)
			.BindBuffer(*uniformBuffers[i], 0, sizeof(UniformBufferObject))
			.BindImage(*textureSampler, albedoTexture.get(), *defaultWhiteTexture)
			.BindImage(*textureSampler, normalTexture.get(), *defaultNormalTexture)
			.BindImage(*textureSampler, metallicTexture.get(), *defaultWhiteTexture)
			.BindImage(*textureSampler, roughnessTexture.get(), *defaultWhiteTexture)
			.BindImage(*textureSampler, aoTexture.get(), *defaultWhiteTexture)
			.BindImage(*textureSampler, emissiveTexture.get(), *defaultBlackTexture)
			.BindImage(*shadowSampler, nullptr, *defaultWhiteTexture, ImageLayout::ShaderReadOnly)
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

	shadowDescriptorSetLayout = std::make_unique<DescriptorSetLayout>(&device, shadowLayoutInfo);
}


void core::gpu::Device::Impl::CreateShadowDescriptorSets()
{
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		DescriptorSet(&device, &shadowDescriptorSets, i)
			.BindBuffer(*uniformBuffers[i], 0, sizeof(UniformBufferObject))
			.Update();
	}
}

void core::gpu::Device::Impl::CreateSyncObjects()
{
	imageAvailable.clear();
	renderFinished.clear();
	inFlightFences.clear();
	imagesInFlight.clear();
	tempCmdBufs.clear();

	vk::SemaphoreCreateInfo semInfo{};
	imageAvailable.reserve(MAX_FRAMES_IN_FLIGHT);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		imageAvailable.emplace_back(device, semInfo);
	}

	uint32_t swapchainImageCount = swapchain->GetImages().size();
	renderFinished.reserve(swapchainImageCount);
	for (size_t i = 0; i < swapchainImageCount; ++i)
	{
		renderFinished.emplace_back(device, semInfo);
	}

	vk::FenceCreateInfo fenceInfo{ .flags = vk::FenceCreateFlagBits::eSignaled };
	inFlightFences.reserve(MAX_FRAMES_IN_FLIGHT);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		inFlightFences.emplace_back(device, fenceInfo);
	}

	imagesInFlight.resize(swapchainImageCount, nullptr);
	tempCmdBufs.resize(MAX_FRAMES_IN_FLIGHT);
}

void core::gpu::Device::Impl::BeginFrame(uint32_t frameIndex)
{
	if (frameIndex >= inFlightFences.size()) return;

	device.waitForFences(*inFlightFences[frameIndex], VK_TRUE, UINT64_MAX);

	if (frameIndex < tempCmdBufs.size())
	{
		tempCmdBufs[frameIndex].reset();
	}
}


uint32_t core::gpu::Device::Impl::AcquireNextImage(uint32_t frameIndex)
{
	if (frameIndex >= imageAvailable.size()) return UINT32_MAX;

	uint32_t imageIndex = UINT32_MAX;
	vk::SwapchainKHR swapchainHandle = reinterpret_cast<VkSwapchainKHR>(swapchain->GetHandle());

	vk::ResultValue<uint32_t> res = device.acquireNextImage2KHR(
		vk::AcquireNextImageInfoKHR{
			.swapchain = swapchainHandle,
			.timeout = UINT64_MAX,
			.semaphore = *imageAvailable[frameIndex],
			.fence = nullptr,
			.deviceMask = 1
		}
	);

	if (res.result == vk::Result::eErrorOutOfDateKHR)
	{
		return UINT32_MAX;
	}

	if (res.result != vk::Result::eSuccess && res.result != vk::Result::eSuboptimalKHR)
	{
		std::cerr << "AcquireNextImage failed: " << vk::to_string(res.result) << "\n";
		return UINT32_MAX;
	}

	imageIndex = res.value;

	if (imageIndex < imagesInFlight.size() && imagesInFlight[imageIndex] != nullptr)
	{
		device.waitForFences(*(*imagesInFlight[imageIndex]), VK_TRUE, UINT64_MAX);
	}

	device.resetFences(*inFlightFences[frameIndex]);

	imagesInFlight[imageIndex] = &inFlightFences[frameIndex];

	return imageIndex;
}

void* core::gpu::Device::Impl::GetImageAvailableSemaphore(uint32_t frameIndex) const
{
	if (frameIndex >= imageAvailable.size()) return nullptr;
	return reinterpret_cast<void*>(static_cast<VkSemaphore>(*imageAvailable[frameIndex]));
}

void* core::gpu::Device::Impl::GetRenderFinishedSemaphore(uint32_t imageIndex) const
{
	if (imageIndex >= renderFinished.size()) return nullptr;
	return reinterpret_cast<void*>(static_cast<VkSemaphore>(*renderFinished[imageIndex]));
}

void* core::gpu::Device::Impl::GetInFlightFence(uint32_t frameIndex) const
{
	if (frameIndex >= inFlightFences.size()) return nullptr;
	return reinterpret_cast<void*>(static_cast<VkFence>(*inFlightFences[frameIndex]));
}

void* core::gpu::Device::Impl::GetSwapchainImage(uint32_t imageIndex) const
{
	return swapchain->GetImage(imageIndex).image;
}

void* core::gpu::Device::Impl::GetColorImage() const
{
	return colorImage ? colorImage->GetHandle() : nullptr;
}

void core::gpu::Device::Impl::TransitionImageForPresent(uint32_t frameIndex, uint32_t imageIndex)
{
	SwapchainImage swapImageStruct = swapchain->GetImage(imageIndex);

	vk::Image swapImage = VK_NULL_HANDLE;
	if (swapImageStruct.image)
	{
		vk::Image* imagePtr = static_cast<vk::Image*>(swapImageStruct.image);
		if (imagePtr)
		{
			swapImage = *imagePtr;
		}
	}

	if (swapImage == VK_NULL_HANDLE)
	{
		std::cerr << "Invalid swapchain image handle!\n";
		return;
	}

	void* poolPtr = commandPool->GetHandle();
	vk::raii::CommandPool* raiiPool = static_cast<vk::raii::CommandPool*>(poolPtr);
	vk::CommandPool poolHandle = **raiiPool;

	vk::CommandBufferAllocateInfo allocInfo{
		.commandPool = poolHandle,
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
			.image = swapImage,
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

void core::gpu::Device::Impl::Present(uint32_t imageIndex)
{
	if (imageIndex >= renderFinished.size()) return;

	vk::Semaphore presentWait = *renderFinished[imageIndex];
	vk::SwapchainKHR vkSwapchain = reinterpret_cast<VkSwapchainKHR>(swapchain->GetHandle());
	;

	vk::PresentInfoKHR presentInfo{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &presentWait,
		.swapchainCount = 1,
		.pSwapchains = &vkSwapchain,
		.pImageIndices = &imageIndex
	};

	try
	{
		vk::Result result = graphicsQueue.presentKHR(presentInfo);
		if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
		{
		}
	}
	catch (const vk::OutOfDateKHRError&)
	{
	}
}

void core::gpu::Device::Impl::CreateColorImage()
{
	ImageCreateInfo colorInfo{
		.width = swapchain->GetWidth(),
		.height = swapchain->GetHeight(),
		.mipLevels = 1,
		.format = swapchain->GetFormat(),
		.tiling = ImageTiling::Optimal,
		.usage = ImageUsage::ColorAttachment | ImageUsage::TransferSrc,
		.memoryProperties = MemoryProperty::DeviceLocal,
		.samples = SampleCount::e4
	};

	colorImage = std::make_unique<Image>(&device, &physicalDevice, colorInfo);

	ImageViewCreateInfo viewInfo{
		.format = swapchain->GetFormat(),
		.isDepth = false
	};

	colorImage->CreateView(viewInfo);
}

void core::gpu::Device::Impl::Cleanup()
{
	device.waitIdle();

	imageAvailable.clear();
	renderFinished.clear();
	inFlightFences.clear();
	imagesInFlight.clear();

	tempCmdBufs.clear();
}

void core::gpu::Device::Impl::CreateCommandBuffers()
{
	return;
}

core::gpu::Buffer* core::gpu::Device::Impl::GetUniformBuffer(uint32_t frameIndex) const
{
	if (frameIndex >= uniformBuffers.size()) return nullptr;
	return uniformBuffers[frameIndex].get();
}

void* core::gpu::Device::Impl::GetCommandPool() const
{
	return commandPool->GetHandle();
}

void* core::gpu::Device::Impl::GetGraphicsQueue() const
{
	return static_cast<void*>(const_cast<vk::Queue*>(&*graphicsQueue));
}

void* core::gpu::Device::Impl::GetPipeline() const
{
	return graphicsPipeline->GetHandle();
}

void* core::gpu::Device::Impl::GetPipelineLayout() const
{
	return graphicsPipeline->GetLayoutHandle();
}

void* core::gpu::Device::Impl::GetDescriptorSet(uint32_t frameIndex) const
{
	if (frameIndex >= descriptorSets.size()) return nullptr;
	return reinterpret_cast<void*>(static_cast<VkDescriptorSet>(**descriptorSets[frameIndex]));
}

void* core::gpu::Device::Impl::GetDepthImage() const
{
	return shadowMapImage ? shadowMapImage->GetHandle() : nullptr;
}

void* core::gpu::Device::Impl::GetHandle() const
{
	return static_cast<void*>(const_cast<vk::Device*>(&*device));
}

uint32_t core::gpu::Device::Impl::GetSwapchainWidth() const
{
	return swapchain->GetWidth();
}

uint32_t core::gpu::Device::Impl::GetSwapchainHeight() const
{
	return swapchain->GetHeight();
}

void* core::gpu::Device::Impl::GetSwapchainImageView(uint32_t imageIndex) const
{
	return swapchain->GetImage(imageIndex).imageView;
}

void* core::gpu::Device::Impl::GetDepthImageView() const
{
	return shadowMapImage->GetViewHandle();
}

void* core::gpu::Device::Impl::GetColorImageView() const
{
	if (!colorImage) return nullptr;
	return colorImage->GetViewHandle();
}

void* core::gpu::Device::Impl::GetPhysicalDevice() const
{
	return static_cast<void*>(const_cast<vk::PhysicalDevice*>(&*physicalDevice));
}

void core::gpu::Device::Impl::WaitIdle()
{
	device.waitIdle();
}

void core::gpu::Device::BeginFrame(uint32_t frameIndex)
{
	if (m_impl) m_impl->BeginFrame(frameIndex);
}

uint32_t core::gpu::Device::AcquireNextImage(uint32_t frameIndex)
{
	return m_impl ? m_impl->AcquireNextImage(frameIndex) : UINT32_MAX;
}

void* core::gpu::Device::GetImageAvailableSemaphore(uint32_t frameIndex) const
{
	return m_impl ? m_impl->GetImageAvailableSemaphore(frameIndex) : nullptr;
}

void* core::gpu::Device::GetRenderFinishedSemaphore(uint32_t imageIndex) const
{
	return m_impl ? m_impl->GetRenderFinishedSemaphore(imageIndex) : nullptr;
}

void* core::gpu::Device::GetInFlightFence(uint32_t frameIndex) const
{
	return m_impl ? m_impl->GetInFlightFence(frameIndex) : nullptr;
}

void core::gpu::Device::Present(uint32_t imageIndex)
{
	if (m_impl) m_impl->Present(imageIndex);
}

void core::gpu::Device::Cleanup()
{
	if (m_impl) m_impl->Cleanup();
}

void* core::gpu::Device::GetCommandPool() const
{
	return m_impl ? m_impl->GetCommandPool() : nullptr;
}

void* core::gpu::Device::GetGraphicsQueue() const
{
	return m_impl ? m_impl->GetGraphicsQueue() : nullptr;
}

void* core::gpu::Device::GetPipeline() const
{
	return m_impl ? m_impl->GetPipeline() : nullptr;
}

void* core::gpu::Device::GetPipelineLayout() const
{
	return m_impl ? m_impl->GetPipelineLayout() : nullptr;
}

void* core::gpu::Device::GetDescriptorSet(uint32_t frameIndex) const
{
	return m_impl->GetDescriptorSet(frameIndex);
}

uint32_t core::gpu::Device::GetSwapchainWidth() const
{
	return m_impl ? m_impl->GetSwapchainWidth() : 0;
}

uint32_t core::gpu::Device::GetSwapchainHeight() const
{
	return m_impl ? m_impl->GetSwapchainHeight() : 0;
}

void* core::gpu::Device::GetSwapchainImageView(uint32_t imageIndex) const
{
	return m_impl ? m_impl->GetSwapchainImageView(imageIndex) : nullptr;
}

void* core::gpu::Device::GetDepthImageView() const
{
	return m_impl ? m_impl->GetDepthImageView() : nullptr;
}

void* core::gpu::Device::GetColorImageView() const
{
	return m_impl ? m_impl->GetColorImageView() : nullptr;
}

void* core::gpu::Device::GetPhysicalDevice() const
{
	return m_impl ? m_impl->GetPhysicalDevice() : nullptr;
}

core::gpu::Buffer* core::gpu::Device::GetUniformBuffer(uint32_t frameIndex)
{
	return m_impl ? m_impl->GetUniformBuffer(frameIndex) : nullptr;
}

void* core::gpu::Device::GetHandle() const
{
	return m_impl ? m_impl->GetHandle() : nullptr;
}

void core::gpu::Device::TransitionImageForPresent(uint32_t frameIndex, uint32_t imageIndex)
{
	if (m_impl) m_impl->TransitionImageForPresent(frameIndex, imageIndex);
}

void* core::gpu::Device::GetSwapchainImage(uint32_t imageIndex) const
{
	return m_impl ? m_impl->GetSwapchainImage(imageIndex) : nullptr;
}

void* core::gpu::Device::GetColorImage() const
{
	return m_impl ? m_impl->GetColorImage() : nullptr;
}

void* core::gpu::Device::GetDepthImage() const
{
	return m_impl ? m_impl->GetDepthImage() : nullptr;
}

void core::gpu::Device::WaitIdle()
{
	if (m_impl) m_impl->WaitIdle();
}