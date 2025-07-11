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

	vkDevice = vk::device::VulkanDevice(instance, surface);
	vkDevice.Initialize();
}

void VulkanEngine::run()
{
}

void VulkanEngine::cleanup()
{

}