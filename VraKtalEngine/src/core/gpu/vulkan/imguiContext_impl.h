#pragma once
#include <core/gpu/imguiContext.h>

namespace core::gpu
{
	struct ImguiContext::Impl
	{
	public:
		explicit Impl(Window& _window, Device& _device);
		~Impl();
		
		void CreateContext(Window& _window, Device& _device);

	private:
		std::vector<VkFramebuffer>				imguiFramebuffers					;
		VkDescriptorPool						imguiDescriptorPool = VK_NULL_HANDLE;
		Device*									m_device			= nullptr		;

	};
}
