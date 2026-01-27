
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "../src/core/gpu/vulkan/imguiContext_impl.h"
#include "../src/core/gpu/vulkan/device_impl.h"
#include "../src/core/gpu/vulkan/swapchain_impl.h"
#include "../src/core/gpu/vulkan/commandBuffer_impl.h"


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
	if (imguiDescriptorPool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(*m_device->GetImpl().device, imguiDescriptorPool, nullptr);
    }
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
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
	ImGuizmo::SetRect(0, 0, _device.GetImpl().GetSwapchain()->GetImpl().extent.height, _device.GetImpl().GetSwapchain()->GetImpl().extent.width);
	ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());
}