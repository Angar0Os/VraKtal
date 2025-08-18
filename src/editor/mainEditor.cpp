#include "mainEditor.h"

#include <../imgui/imgui.h>
#include <../imgui/imgui_impl_vulkan.h>
#include <../imgui/imgui_impl_glfw.h>

#include "../core/renderContext_impl_glfw_vulkan.h"
#include "../core/gpu/descriptor_impl_vulkan.h"

using namespace editor;

MainEditor::MainEditor(core::rhi::RenderContext& rCtx)
	: m_rCtx(&rCtx)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForVulkan(rCtx.GetInternal().window, true);

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.ApiVersion = VK_API_VERSION_1_3;
	init_info.Instance = rCtx.GetInternal().instance;
	init_info.PhysicalDevice = rCtx.GetInternal().chosenGPU;
	init_info.Device = rCtx.GetInternal().device;
	init_info.QueueFamily = rCtx.GetInternal().graphicsQueueFamily;
	init_info.Queue = rCtx.GetInternal().graphicsQueue;
	//init_info.DescriptorPool = rCtx.GetInternal().descriptor->GetInternal().globalDescriptorAllocator.pool;
	init_info.MinImageCount = 2;
	init_info.ImageCount = 2;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.UseDynamicRendering = true;

	VkFormat backbufferFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

	init_info.PipelineRenderingCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		.colorAttachmentCount = 1Ui32,
		.pColorAttachmentFormats = &backbufferFormat
	};

	ImGui_ImplVulkan_Init(&init_info);
}

void MainEditor::RunImgui()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Hello, world!");

	ImGui::Text("This is some useful text.");
	ImGui::End();

	ImGui::Render();

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_rCtx->GetInternal().GetCurrentFrame().mainCommandBuffer);
}


void MainEditor::ClearImgui()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
