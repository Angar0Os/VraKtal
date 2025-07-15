#include <vkEngine.h>
#include <vk/commandBuffer.h>

VulkanEngine* loadedEngine = nullptr;

VulkanEngine& VulkanEngine::Get()
{
	return *loadedEngine;
}

VulkanEngine::VulkanEngine()
{
}

VulkanEngine::~VulkanEngine() noexcept
{

}

void VulkanEngine::init()
{
	assert(loadedEngine == nullptr);
	loadedEngine = this;

	rCtx = core::RenderContext({ "VraKtalEngine " });
	vkInstance.init(instance.instance, rCtx.GetWindow(), callBacks, surface, debug_messenger, useValidationLayers);
	vkDevice = vk::VulkanDevice(instance, surface);
	vkDevice.Initialize();

	vkCommandBuffer = vk::VulkanCommandBuffer(immFence, immCommandBuffer, vkDevice.GetVkDevice(), vkDevice.GetGraphicsQueue());
	vkSwapchain = vk::VulkanSwapchain(vkDevice.GetVkPhysicalDevice(), vkDevice.GetVkDevice(), surface, &vkCommandBuffer);
	vkCommandBuffer.Init();
	vkSync.Init(vkCommandBuffer, vkDevice);
	vkDescriptor.Init(vkDevice.GetVkDevice(), &vkCommandBuffer, &vkSwapchain);

}

void VulkanEngine::run()
{
}

void VulkanEngine::cleanup()
{

}