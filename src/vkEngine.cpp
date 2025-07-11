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
	vkInstance.init(instance, rCtx.GetWindow(), callBacks, surface, debug_messenger, useValidationLayers);
}

void VulkanEngine::run()
{
}

void VulkanEngine::cleanup()
{

}