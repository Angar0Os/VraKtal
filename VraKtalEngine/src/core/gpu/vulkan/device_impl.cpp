#define NOMINMAX // Disable Windows min/max macros which conflict with std::min/max
#define LAB_TASK_LEVEL 1

#include "../src/core/gpu/vulkan/device_impl.h"
#include "../src/core/gpu/vulkan/accelerationStructure_impl.h"
#include "../src/core/gpu/vulkan/commandPool_impl.h"
#include "../src/core/gpu/vulkan/image_impl.h"
#include "../src/core/gpu/vulkan/swapchain_impl.h"
#include "../src/core/gpu/vulkan/imguiContext_impl.h"

#include "../src/core/gpu_detail/converters.h"

#include <core/gpu/descriptorSet.h>
#include <core/enum.h>
#include <core/gpu/swapchain.h>

#include <graphics/resources/object/mesh.h>

#include <iostream>
#include <stdexcept>
#include <GLFW/glfw3.h>

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

core::gpu::Device::Impl& core::gpu::Device::GetImpl() const
{
	return *m_impl;
}

core::gpu::Device::Device(core::Window& window)
{
	m_impl = std::make_unique<Impl>(window, nullptr);
	m_impl->parent = this;
	m_impl->Initialize();
	m_imGuiContext = new ImguiContext(window, *this);
}

core::gpu::Device::~Device()
{
	if (m_imGuiContext)
	{
		delete m_imGuiContext;
		m_imGuiContext = nullptr;
	}
}

core::gpu::Device::Impl::Impl(core::Window& _window, const core::gpu::Device* _parent)
	: m_window(_window), parent(_parent)
{
}

void core::gpu::Device::Impl::Initialize()
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

	// Renderer 
	CreateDefaultTextures();
	LoadMaterialTextures();
	CreateColorImage();
	CreateDepthImage();
	CreateGraphicsPipeline();
	CreateDescriptorSets();
}

core::gpu::Device::Impl::~Impl()
{
	descriptorPool.reset();
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

void core::gpu::Device::Impl::CreateLogicalDevice()
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
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain = {
	   {.features = {.samplerAnisotropy = true} },
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

void core::gpu::Device::Impl::CreateDescriptorPool()
{
	descriptorPool = std::make_unique<DescriptorPool>(parent);
}

// Renderer / Faire bindAccelerationStructure dans descriptor Set et changer l'update par Bind + update
void core::gpu::Device::Impl::UpdateDescriptorWithTLAS(uint32_t frameIndex, const core::gpu::AccelerationStructure* tlasHandle)
{
	if (frameIndex >= descriptorSets.size() || !tlasHandle) return;

	vk::DescriptorSet descSet = **descriptorSets[frameIndex];
	vk::AccelerationStructureKHR accelStructHandle = **tlasHandle->GetImpl().accelerationStructure;

	vk::WriteDescriptorSetAccelerationStructureKHR accelInfo{};
	accelInfo.accelerationStructureCount = 1;
	accelInfo.pAccelerationStructures = &accelStructHandle;

	vk::WriteDescriptorSet writeDesc{};
	writeDesc.dstSet = descSet;
	writeDesc.dstBinding = 8;
	writeDesc.dstArrayElement = 0;
	writeDesc.descriptorCount = 1;
	writeDesc.descriptorType = vk::DescriptorType::eAccelerationStructureKHR;
	writeDesc.pNext = &accelInfo;

	device.updateDescriptorSets(writeDesc, nullptr);
}

void core::gpu::Device::Impl::CreateCommandPool()
{
	CommandPoolCreateInfo poolInfo{
		.queueFamilyIndex = queueIndex,
		.flags = CommandPoolCreateFlags::ResetCommandBuffer
	};

	commandPool = std::make_unique<CommandPool>(parent, poolInfo);
}

// Renderer
void core::gpu::Device::Impl::CreateDefaultTextures()
{
	defaultWhiteTexture = std::make_unique<Texture>(parent, commandPool.get(), 1.0f, 1.0f, 1.0f, 1.0f);
	defaultBlackTexture = std::make_unique<Texture>(parent, commandPool.get(), 0.0f, 0.0f, 0.0f, 1.0f);
	defaultNormalTexture = std::make_unique<Texture>(parent, commandPool.get(), 0.5f, 0.5f, 1.0f, 1.0f);
}

// Renderer
void core::gpu::Device::Impl::LoadMaterialTextures()
{
	albedoTexture = std::make_unique<Texture>(parent, commandPool.get(), 1.0f, 1.0f, 1.0f, 1.0f);
	normalTexture = std::make_unique<Texture>(parent, commandPool.get(), 0.5f, 0.5f, 1.0f, 1.0f);
	metallicTexture = std::make_unique<Texture>(parent, commandPool.get(), 1.0f, 1.0f, 1.0f, 1.0f);
	roughnessTexture = std::make_unique<Texture>(parent, commandPool.get(), 1.0f, 1.0f, 1.0f, 1.0f);
	aoTexture = std::make_unique<Texture>(parent, commandPool.get(), 1.0f, 1.0f, 1.0f, 1.0f);
	emissiveTexture = std::make_unique<Texture>(parent, commandPool.get(), 0.0f, 0.0f, 0.0f, 1.0f);

	/* Loader une image via un path puis creer une texture*/

	/*albedoTexture->LoadTextureIfExists(parent, "../bin/assets/textures/viking_room.png");
	normalTexture->LoadTextureIfExists(parent, "assets/textures/normal.png");
	metallicTexture->LoadTextureIfExists(parent, "assets/textures/metallic.png");
	roughnessTexture->LoadTextureIfExists(parent, "assets/textures/roughness.png");
	aoTexture->LoadTextureIfExists(parent, "assets/textures/ao.png");
	emissiveTexture->LoadTextureIfExists(parent, "assets/textures/emissive.png");*/
}

void core::gpu::Device::Impl::CreateSwapchain()
{
	int width, height = 0;
	glfwGetFramebufferSize(m_window.GlfwHandle(), &width, &height);

	SwapchainCreateInfo swapchainInfo{
		.surface = static_cast<void*>(static_cast<VkSurfaceKHR>(*surface)),
		.width = static_cast<uint32_t>(width),
		.height = static_cast<uint32_t>(height),
		.preferredFormat = TextureFormat::RGBA8_SRGB,
		.presentMode = PresentMode::Mailbox,
		.minImageCount = 3,
		.oldSwapchain = nullptr
	};

	swapchain = std::make_unique<Swapchain>(parent, swapchainInfo);
}

//Renderer
void core::gpu::Device::Impl::CreateGraphicsPipeline()
{
	auto shaderCode = ReadFile("../bin/assets/shaders/slang.spv");

	SVertexInputBinding vertexBinding{
		.binding = 0,
		.stride = sizeof(graphics::resources::Vertex),
		.inputRate = VertexInputRate::Vertex
	};

	std::vector<SVertexInputAttribute> vertexAttributes = {
		{0, 0, TextureFormat::RGB32_Float, offsetof(graphics::resources::Vertex, position)},
		{1, 0, TextureFormat::RGB32_Float, offsetof(graphics::resources::Vertex, normal)},
		{2, 0, TextureFormat::RG32_Float, offsetof(graphics::resources::Vertex, uv)}
	};

	std::vector<ShaderStage> shaderStages = {
		{ShaderStageFlags::Vertex, shaderCode, "vertMain"},
		{ShaderStageFlags::Fragment, shaderCode, "fragMain"}
	};

	std::vector<PushConstantRange> pushConstants = {
		{
			.stageFlags = static_cast<uint32_t>(ShaderStageFlags::Vertex),
			.offset = 0,
			.size = sizeof(glm::mat4)
		}
	};

	PipelineCreateInfo pipelineInfo{};
	pipelineInfo.shaderStages = shaderStages;
	pipelineInfo.vertexBindings = { vertexBinding };
	pipelineInfo.vertexAttributes = vertexAttributes;
	pipelineInfo.topology = PrimitiveTopology::TriangleList;
	pipelineInfo.polygonMode = PolygonMode::Fill;
	pipelineInfo.cullMode = CullMode::None;
	pipelineInfo.frontFace = FrontFace::Clockwise;
	pipelineInfo.depthTestEnable = true;
	pipelineInfo.depthWriteEnable = true;
	pipelineInfo.depthCompareOp = CompareOp::Less;
	pipelineInfo.blendEnable = false;
	pipelineInfo.samples = SampleCount::e4;
	pipelineInfo.colorAttachmentFormats = { core::gpu_detail::FromVulkan(swapchain->GetImpl().format) };
	pipelineInfo.depthAttachmentFormat = TextureFormat::Depth32F;
	pipelineInfo.descriptorSetLayouts = { descriptorSetLayout.get() };
	pipelineInfo.pushConstantRanges = pushConstants;
	pipelineInfo.dynamicStates = { DynamicState::Viewport, DynamicState::Scissor };

	graphicsPipeline = std::make_unique<Pipeline>(parent, pipelineInfo);
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
	int width = 0, height = 0;
	glfwGetFramebufferSize(m_window.GlfwHandle(), &width, &height);

	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(m_window.GlfwHandle(), &width, &height);
		glfwWaitEvents();
	}

	device.waitIdle();

	imageAvailable.clear();
	renderFinished.clear();
	inFlightFences.clear();
	imagesInFlight.clear();
	tempCmdBufs.clear();

	SwapchainCreateInfo swapchainInfo{
		.surface = static_cast<void*>(static_cast<VkSurfaceKHR>(*surface)),
		.width = static_cast<uint32_t>(width),
		.height = static_cast<uint32_t>(height),
		.preferredFormat = TextureFormat::RGBA8_SRGB,
		.presentMode = PresentMode::Mailbox,
		.minImageCount = 3,
		.oldSwapchain = swapchain.get()
	};
	swapchain = std::make_unique<Swapchain>(parent, swapchainInfo);

	// Renderer
	CreateColorImage();
	CreateDepthImage();
	CreateGraphicsPipeline();
	CreateSyncObjects();
	CreateDescriptorSets();
}

// Renderer
void core::gpu::Device::Impl::CreateDescriptorSets()
{
	//for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	//{
	//	DescriptorSet(parent, &descriptorSets, i)
	//		.BindBuffer(*uniformBuffers[i], 0, sizeof(UniformBufferObject))
	//		.BindImage(*textureSampler, albedoTexture.get(), *defaultWhiteTexture)
	//		.BindImage(*textureSampler, normalTexture.get(), *defaultNormalTexture)
	//		.BindImage(*textureSampler, metallicTexture.get(), *defaultWhiteTexture)
	//		.BindImage(*textureSampler, roughnessTexture.get(), *defaultWhiteTexture)
	//		.BindImage(*textureSampler, aoTexture.get(), *defaultWhiteTexture)
	//		.BindImage(*textureSampler, emissiveTexture.get(), *defaultBlackTexture)
	//		.Update();
	//}
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

	uint32_t swapchainImageCount = swapchain->GetImpl().images.size();
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
	if (frameIndex >= inFlightFences.size())
	{
		return;
	}

	device.waitForFences(*inFlightFences[frameIndex], VK_TRUE, UINT64_MAX);
	device.resetFences(*inFlightFences[frameIndex]);

	if (frameIndex < tempCmdBufs.size())
	{
		tempCmdBufs[frameIndex].reset();
	}
}


uint32_t core::gpu::Device::Impl::AcquireNextImage(uint32_t frameIndex)
{
	if (frameIndex >= imageAvailable.size())
	{
		return UINT32_MAX;
	}

	try
	{
		vk::ResultValue<uint32_t> result = device.acquireNextImage2KHR(
			vk::AcquireNextImageInfoKHR{
				.swapchain = swapchain->GetImpl().swapchain,
				.timeout = UINT64_MAX,
				.semaphore = *imageAvailable[frameIndex],
				.fence = nullptr,
				.deviceMask = 1
			}
		);

		uint32_t imageIndex = result.value;

		if (result.result == vk::Result::eErrorOutOfDateKHR)
		{
			return UINT32_MAX;
		}

		if (result.result != vk::Result::eSuccess && result.result != vk::Result::eSuboptimalKHR)
		{
			return UINT32_MAX;
		}

		if (imageIndex < imagesInFlight.size() && imagesInFlight[imageIndex] != nullptr)
		{
			device.waitForFences(**imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}

		imagesInFlight[imageIndex] = &inFlightFences[frameIndex];

		return imageIndex;
	}
	catch (const vk::OutOfDateKHRError&)
	{
		return UINT32_MAX;
	}
	catch (const vk::SystemError& e)
	{
		return UINT32_MAX;
	}
}

void* core::gpu::Device::Impl::GetImageAvailableSemaphore(uint32_t frameIndex) const
{
	if (frameIndex >= imageAvailable.size()) return nullptr;
	return reinterpret_cast<void*>(static_cast<VkSemaphore>(*imageAvailable[frameIndex]));
}

void* core::gpu::Device::Impl::GetRenderFinishedSemaphore(uint32_t imageIndex) const
{
	if (imageIndex >= renderFinished.size()) return nullptr;
	return static_cast<void*>(static_cast<VkSemaphore>(*renderFinished[imageIndex]));
}

void* core::gpu::Device::Impl::GetInFlightFence(uint32_t frameIndex) const
{
	if (frameIndex >= inFlightFences.size()) return nullptr;
	return static_cast<void*>(static_cast<VkFence>(*inFlightFences[frameIndex]));
}

const core::gpu::Swapchain* core::gpu::Device::Impl::GetSwapchain() const
{
	return swapchain.get();
}

const core::gpu::Image* core::gpu::Device::Impl::GetSwapchainImage(uint32_t imageIndex) const
{
	if (!swapchain)
	{
		std::cerr << "ERROR: Swapchain is null!" << std::endl;
		return nullptr;
	}

	if (imageIndex >= swapchain->GetImpl().images.size())
	{
		std::cerr << "ERROR: Image index " << imageIndex
			<< " out of range (swapchain has "
			<< swapchain->GetImpl().images.size() << " images)" << std::endl;
		return nullptr;
	}

	return swapchain->GetImpl().images[imageIndex].get();
}

const core::gpu::Image* core::gpu::Device::Impl::GetColorImage() const
{
	return colorImage.get();
}

void core::gpu::Device::Impl::TransitionImageForPresent(uint32_t frameIndex, uint32_t imageIndex)
{
	const core::gpu::Image* swapchainImage = swapchain->GetImpl().images[imageIndex].get();

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
			.image = swapchainImage->GetImpl().GetVkImage(),
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
	vk::SwapchainKHR swapchainHandle = swapchain->GetImpl().swapchain;

	vk::PresentInfoKHR presentInfo{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &presentWait,
		.swapchainCount = 1,
		.pSwapchains = &swapchainHandle,
		.pImageIndices = &imageIndex
	};
	try
	{
		vk::Result result = graphicsQueue.presentKHR(presentInfo);
	}
	catch (const vk::OutOfDateKHRError&)
	{
	}
	catch (const vk::SystemError& e)
	{
	}
}


// Renderer
void core::gpu::Device::Impl::CreateColorImage()
{
	SImageCreateInfo colorInfo{
		.width = swapchain->GetImpl().extent.width,
		.height = swapchain->GetImpl().extent.height,
		.mipLevels = 1,
		.format = core::gpu_detail::FromVulkan(swapchain->GetImpl().format),
		.tiling = ImageTiling::Optimal,
		.usage = ImageUsage::ColorAttachment | ImageUsage::TransferSrc,
		.memoryProperties = EMemoryProperty::DeviceLocal,
		.samples = SampleCount::e4
	};

	colorImage = std::make_unique<Image>(parent, colorInfo);

	SImageViewCreateInfo viewInfo{
		.format = core::gpu_detail::FromVulkan(swapchain->GetImpl().format),
		.isDepth = false
	};

	colorImage->CreateView(viewInfo);
}

// Renderer
void core::gpu::Device::Impl::CreateDepthImage()
{
	SImageCreateInfo depthInfo{
		.width = swapchain->GetImpl().extent.width,
		.height = swapchain->GetImpl().extent.height,
		.mipLevels = 1,
		.format = TextureFormat::Depth32F,
		.tiling = ImageTiling::Optimal,
		.usage = ImageUsage::DepthStencilAttachment,
		.memoryProperties = EMemoryProperty::DeviceLocal,
		.samples = SampleCount::e4
	};

	depthImage = std::make_unique<Image>(parent, depthInfo);

	SImageViewCreateInfo viewInfo{
		.format = TextureFormat::Depth32F,
		.isDepth = true
	};

	depthImage->CreateView(viewInfo);
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

const core::gpu::Pipeline* core::gpu::Device::Impl::GetGraphicsPipeline() const
{
	return graphicsPipeline.get();
}

const core::gpu::Image* core::gpu::Device::Impl::GetDepthImage() const
{
	return depthImage.get();
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

core::gpu::Buffer* core::gpu::Device::GetUniformBuffer(uint32_t frameIndex)
{
	return m_impl ? m_impl->GetUniformBuffer(frameIndex) : nullptr;
}

void core::gpu::Device::TransitionImageForPresent(uint32_t frameIndex, uint32_t imageIndex)
{
	if (m_impl) m_impl->TransitionImageForPresent(frameIndex, imageIndex);
}

const core::gpu::Image* core::gpu::Device::GetSwapchainImage(uint32_t imageIndex) const
{
	return m_impl ? m_impl->GetSwapchainImage(imageIndex) : nullptr;
}

const core::gpu::Image* core::gpu::Device::GetColorImage() const
{
	return m_impl ? m_impl->GetColorImage() : nullptr;
}

const core::gpu::Image* core::gpu::Device::GetDepthImage() const
{
	return m_impl ? m_impl->GetDepthImage() : nullptr;
}

void core::gpu::Device::WaitIdle()
{
	if (m_impl) m_impl->WaitIdle();
}

void core::gpu::Device::RecreateSwapchain()
{
	if (m_impl) m_impl->RecreateSwapchain();
}

const core::gpu::Pipeline* core::gpu::Device::GetGraphicsPipeline() const
{
	return m_impl ? m_impl->GetGraphicsPipeline() : nullptr;
}

void core::gpu::Device::UpdateDescriptorWithTLAS(uint32_t frameIndex, const core::gpu::AccelerationStructure* tlasHandle)
{
	if (m_impl) m_impl->UpdateDescriptorWithTLAS(frameIndex, tlasHandle);
}

core::gpu::ImguiContext* core::gpu::Device::GetImGuiContext()
{
	return m_imGuiContext;
}

