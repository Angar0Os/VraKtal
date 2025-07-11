#include <vkEngine.h>

VulkanEngine::VulkanEngine()
{

}

VulkanEngine::~VulkanEngine() noexcept
{

}

void VulkanEngine::init()
{
	rCtx = vk::core::RenderContext({ "VraKtalEngine " });
}

void VulkanEngine::run()
{
}

void VulkanEngine::cleanup()
{

}