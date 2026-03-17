#pragma once
#include <core/gpu/imguiContext.h>
#include <core/gpu/image.h>
#include <graphics/renderer.h>

namespace core::gpu
{
	struct ImguiContext::Impl
	{
	public:
		explicit Impl(Window& _window, Device& _device);
		~Impl();
		
		void CreateContext(Window& _window, Device& _device);

		void InitViewport(uint32_t width, uint32_t height);
		void ResizeViewport(uint32_t width, uint32_t height);
		void DestroyViewport();
		void EnsureViewport(uint32_t width, uint32_t height);

		void SyncViewportResources();
		void SetDesiredViewportSize(uint32_t width, uint32_t height);
		void DrawViewportComponent(uint32_t width, uint32_t height);
		void RenderSceneToViewport(core::gpu::CommandBuffer* cmd, graphics::Renderer* renderer);

		struct SceneViewportImpl : ImguiContext::SceneViewport {
			std::unique_ptr<Image> colorImage = nullptr;
			vk::raii::Sampler sampler = nullptr;
			VkDescriptorSet imguiDescriptorSet = VK_NULL_HANDLE;
		};

		void OnResize();
		SceneViewportImpl m_viewport;

	private:
		std::vector<VkFramebuffer>				imguiFramebuffers					;
		VkDescriptorPool						imguiDescriptorPool = VK_NULL_HANDLE;
		Device*									m_device			= nullptr		;
	};
}