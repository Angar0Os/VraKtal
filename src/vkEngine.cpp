#include <vkEngine.h>

VulkanEngine::VulkanEngine()
{
}

VulkanEngine::~VulkanEngine() noexcept
{

}

void VulkanEngine::init()
{
	rCtx = core::RenderContext({ "VraKtalEngine " });
	vkInstance.init(instance.instance, rCtx.GetWindow(), callBacks, surface, debug_messenger, useValidationLayers);
	vkDevice = vk::VulkanDevice(instance, surface);
	vkDevice.Initialize();

	vkCommandBuffer = vk::VulkanCommandBuffer(immFence, immCommandBuffer, vkDevice.GetVkDevice(), vkDevice.GetGraphicsQueue());
	vkSwapchain = vk::VulkanSwapchain(vkDevice.GetVkPhysicalDevice(), vkDevice.GetVkDevice(), surface, vkCommandBuffer);
	
}

void VulkanEngine::run()
{
}

void VulkanEngine::cleanup()
{

}