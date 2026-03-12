
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "../src/core/gpu/vulkan/imguiContext_impl.h"
#include "../src/core/gpu/vulkan/image_impl.h"
#include "../src/core/gpu/vulkan/device_impl.h"
#include "../src/core/gpu/vulkan/swapchain_impl.h"
#include "../src/core/gpu/vulkan/commandBuffer_impl.h"
#include "../src/core/gpu_detail/converters.h"

#include <core/window.h>
#include <core/gpu/device.h>
#include <graphics/renderer.h>


#include <core/gpu/commandBuffer.h>

#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"
#include "imGuizmo/ImGuizmo.h"

#include <glm/gtc/type_ptr.inl>

#include "MDI/IconsMaterialDesignIcons.h"

#include <iostream>
#include <vulkan/vulkan_handles.hpp>


core::gpu::ImguiContext::ImguiContext(Window& _window, Device& _device)
    : m_impl(std::make_unique<Impl>(_window, _device))
{}

core::gpu::ImguiContext::~ImguiContext() {}

void core::gpu::ImguiContext::PrepareDrawData()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

    m_prepareDrawDataFunc();

	ImGui::Render();
}

void core::gpu::ImguiContext::DrawEditors(void* _commandBuffer )
{
	CommandBuffer* commandBuffer = static_cast<CommandBuffer*>(_commandBuffer);
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData() , static_cast<VkCommandBuffer>(*commandBuffer->GetImpl().GetCommandBuffer(0)));
}

void core::gpu::ImguiContext::PrepareForDrawing()
{
	m_impl->SyncViewportResources();
}

void core::gpu::ImguiContext::BindPrepareDrawData(std::function<void()> func)
{
    m_prepareDrawDataFunc = func;
}

core::gpu::ImguiContext::Impl::Impl(Window& _window, Device& _device)
{
    CreateContext(_window, _device);
}

core::gpu::ImguiContext::Impl::~Impl()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	if(imguiDescriptorPool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(*m_device->GetImpl().device, imguiDescriptorPool, nullptr);
		imguiDescriptorPool = VK_NULL_HANDLE;
	}
}

void core::gpu::ImguiContext::Impl::CreateContext(Window& _window, Device& _device)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGuiIO& io = ImGui::GetIO();

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	io.Fonts->AddFontDefault();
	static const ImWchar icons_ranges[] = { ICON_MIN_MDI, ICON_MAX_MDI, 0 };

	// Small icons (in default font)
	ImFontConfig icons_config_small;
	icons_config_small.MergeMode = true;
	icons_config_small.PixelSnapH = true;
	icons_config_small.GlyphMinAdvanceX = 13.0f;
	io.Fonts->AddFontFromFileTTF("../external/fonts/" FONT_ICON_FILE_NAME_MDI, 13.0f, &icons_config_small, icons_ranges);

	// Large icons
	ImFontConfig icons_config_large;
	icons_config_large.PixelSnapH = true;
	icons_config_large.GlyphMinAdvanceX = 50.0f;
	io.Fonts->AddFontFromFileTTF("../external/fonts/" FONT_ICON_FILE_NAME_MDI, 50.0f, &icons_config_large, icons_ranges);

	// Initialize GLFW 
	ImGui_ImplGlfw_InitForVulkan(_window.GlfwHandle(), true);

	// Create a large descriptor pool for ImGui usage (it uses many descriptor types)
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
	pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	if (vkCreateDescriptorPool(*_device.GetImpl().device, &pool_info, nullptr, &imguiDescriptorPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create ImGui descriptor pool");

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = *_device.GetImpl().instance;
	init_info.PhysicalDevice = *_device.GetImpl().physicalDevice;
	init_info.Device = *_device.GetImpl().device;
	init_info.QueueFamily = _device.GetImpl().queueIndex;
	init_info.Queue = *_device.GetImpl().graphicsQueue;
	init_info.DescriptorPool = imguiDescriptorPool;
	init_info.MinImageCount = 2;
	init_info.ImageCount = _device.GetImpl().GetSwapchain()->GetImpl().images.size();
	init_info.MSAASamples = VK_SAMPLE_COUNT_4_BIT;
	init_info.RenderPass = VK_NULL_HANDLE;
    init_info.UseDynamicRendering = VK_TRUE;
    init_info.PipelineRenderingCreateInfo = {};
    init_info.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
	VkFormat format = static_cast<VkFormat>(_device.GetImpl().GetSwapchain()->GetImpl().format);
	init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &format;
	init_info.PipelineRenderingCreateInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;

	ImGui_ImplVulkan_Init(&init_info);
	ImGuizmo::SetRect(0, 0, (float)_device.GetImpl().GetSwapchain()->GetImpl().extent.width, (float)_device.GetImpl().GetSwapchain()->GetImpl().extent.height);
	ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());

	m_device = &_device;
}

void core::gpu::ImguiContext::Impl::InitViewport(uint32_t width, uint32_t height)
{
	auto& vkDevice = m_device->GetImpl().device;

	m_viewport.width = width;
	m_viewport.height = height;

	core::gpu::SImageCreateInfo imageInfo{};
	imageInfo.width = width;
	imageInfo.height = height;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = gpu_detail::FromVulkan(m_device->GetImpl().GetSwapchain()->GetImpl().format);
	imageInfo.samples = SampleCount::e1;
	imageInfo.tiling = ImageTiling::Optimal;
	imageInfo.usage = ImageUsage::ColorAttachment | ImageUsage::Sampled;
	imageInfo.memoryProperties = EMemoryProperty::DeviceLocal;

	m_viewport.colorImage = std::make_unique<Image>(m_device, imageInfo);

	core::gpu::SImageViewCreateInfo viewInfo{};
	viewInfo.format = imageInfo.format;
	viewInfo.isDepth = false;
	viewInfo.baseMipLevel = 0;
	viewInfo.levelCount = 1;
	viewInfo.baseArrayLayer = 0;
	viewInfo.layerCount = 1;

	m_viewport.colorImage->CreateView(viewInfo);

	vk::SamplerCreateInfo samplerInfo{};
	samplerInfo.magFilter = vk::Filter::eLinear;
	samplerInfo.minFilter = vk::Filter::eLinear;
	samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
	samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
	samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
	samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 1.0f;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = vk::CompareOp::eAlways;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 1.0f;
	samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;

	m_viewport.sampler = vk::raii::Sampler(vkDevice, samplerInfo);

	// Transition si non ca crash
	{
		core::gpu::SCommandBufferCreateInfo cmdInfo{};
		cmdInfo.device = m_device;
		cmdInfo.count = 1;
		cmdInfo.singleTime = true;
		cmdInfo.level = core::ECommandBufferLevel::Primary;

		core::gpu::CommandBuffer cmd(m_device, cmdInfo);
		cmd.Begin(0);

		cmd.TransitionImageLayout(
			m_viewport.colorImage.get(),
			core::ImageLayout::Undefined,
			core::ImageLayout::ShaderReadOnly,
			false
		);

		cmd.End(0);
		cmd.SubmitAndWait(m_device);
	}

	m_viewport.imguiDescriptorSet = ImGui_ImplVulkan_AddTexture(
		static_cast<VkSampler>(*m_viewport.sampler),
		static_cast<VkImageView>(*m_viewport.colorImage->GetImpl().view),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	);

	m_viewport.readyForUse = true;
}
void core::gpu::ImguiContext::DrawViewportComponent(uint32_t width , uint32_t height)
{
	m_impl->DrawViewportComponent(width , height);
}

void core::gpu::ImguiContext::RenderSceneToViewport(core::gpu::CommandBuffer* cmd, graphics::Renderer* renderer)
{
	m_impl->RenderSceneToViewport(cmd, renderer);
}

void core::gpu::ImguiContext::Impl::SyncViewportResources()
{
	if (m_viewport.desiredWidth == 0 || m_viewport.desiredHeight == 0)
		return;

	if (!m_viewport.colorImage)
	{
		InitViewport(m_viewport.desiredWidth, m_viewport.desiredHeight);
	}
	else if (m_viewport.width != m_viewport.desiredWidth || m_viewport.height != m_viewport.desiredHeight)
	{
		ResizeViewport(m_viewport.desiredWidth, m_viewport.desiredHeight);
	}
}

void core::gpu::ImguiContext::Impl::SetDesiredViewportSize(uint32_t width, uint32_t height)
{
	if (width >= m_device->GetImpl().physicalDevice.getProperties().limits.maxFramebufferWidth - 10 || height >= m_device->GetImpl().physicalDevice.getProperties().limits.maxFramebufferHeight - 10)
		return; //Imgui return max si la fenetre a width ou height a 0

	m_viewport.desiredWidth = width;
	m_viewport.desiredHeight = height;
}

void core::gpu::ImguiContext::Impl::DrawViewportComponent(uint32_t width, uint32_t height)
{
	SetDesiredViewportSize(width, height);

	if (m_viewport.imguiDescriptorSet != VK_NULL_HANDLE)
	{
		ImGui::Image(
			(ImTextureID)m_viewport.imguiDescriptorSet,
			ImVec2((float)width, (float)height),
			ImVec2(0, 0),
			ImVec2(1, 1)
		);
	}
}

void core::gpu::ImguiContext::Impl::RenderSceneToViewport(core::gpu::CommandBuffer* cmd, graphics::Renderer* renderer)
{
	if (!cmd || !renderer)
		return;

	if(!m_viewport.colorImage || !m_viewport.readyForUse)
		return;

	if (m_viewport.width <= 1 || m_viewport.height <= 1)
		return;

	//On doit ajuster la camera
	const float aspectRatio = static_cast<float>(m_viewport.width) / static_cast<float>(m_viewport.height);

	glm::mat4 projection = glm::perspectiveLH_ZO(
		glm::radians(45.0f),
		aspectRatio,
		0.1f,
		100.0f
	);
	projection[1][1] *= -1;

	//TODO CAMERA POS SHOULD BE IN ECS IN THE FUTURE
	glm::vec3 cameraPosition = glm::vec3(0.0f, 3.0f, -5.0f);

	glm::mat4 view = glm::lookAtLH(
		cameraPosition,
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	renderer->SetCamera(view, projection);

	cmd->TransitionImageLayout(
		m_viewport.colorImage.get(),
		core::ImageLayout::ShaderReadOnly,
		core::ImageLayout::ColorAttachment,
		false
	);

	cmd->BeginRendering(
		m_device,
		m_viewport.colorImage.get(),
		nullptr
	);

	cmd->SetViewport(
	0.0f,
	0.0f,
	static_cast<float>(m_viewport.width),
	static_cast<float>(m_viewport.height),
	0.0f,
	1.0f
);

	cmd->SetScissor(
		0,
		0,
		m_viewport.width,
		m_viewport.height
	);

	cmd->BindPipeline(m_device->GetGraphicsPipeline());
	cmd->BindDescriptorSets(m_device, renderer->GetCurrentFrame(), 0);

	renderer->DrawScene(cmd);
	cmd->EndRendering();

	cmd->TransitionImageLayout(
		m_viewport.colorImage.get(),
		core::ImageLayout::ColorAttachment,
		core::ImageLayout::ShaderReadOnly,
		false
	);
}

void core::gpu::ImguiContext::Impl::ResizeViewport(uint32_t width, uint32_t height)
{
	m_device->WaitIdle();
	DestroyViewport();
	InitViewport(width, height);
}

void core::gpu::ImguiContext::Impl::DestroyViewport()
{
	if (m_viewport.imguiDescriptorSet != VK_NULL_HANDLE)
	{
		ImGui_ImplVulkan_RemoveTexture(m_viewport.imguiDescriptorSet);
		m_viewport.imguiDescriptorSet = VK_NULL_HANDLE;
	}

	m_viewport.sampler = nullptr;
	m_viewport.colorImage.reset();

	m_viewport.width = 0;
	m_viewport.height = 0;
	m_viewport.readyForUse = false;
}

void core::gpu::ImguiContext::Impl::EnsureViewport(uint32_t width, uint32_t height)
{
	if (width == 0 || height == 0)
		return;

	if (!m_viewport.colorImage)
	{
		InitViewport(width, height);
	}
	else if (m_viewport.width != width || m_viewport.height != height)
	{
		ResizeViewport(width, height);
	}
}
