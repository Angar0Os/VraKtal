#pragma once
#include <memory>
#include <iostream>
#include <functional>

namespace graphics {
	class Renderer;
}

namespace core
{
	class Window;
	namespace gpu
	{
        class Device;
        class CommandBuffer;
		class Image;

		struct SceneViewport;

		class ImguiContext
		{
		private:
			struct Impl;

			std::unique_ptr<Impl> m_impl;
            std::function<void()> m_prepareDrawDataFunc;
			struct SceneViewport
			{
				uint32_t width = 0;
				uint32_t height = 0;
				uint32_t desiredWidth = 0;
				uint32_t desiredHeight = 0;
				bool firstFrame = true;
				bool readyForUse = false;
			};
			std::unique_ptr<SceneViewport> m_SceneViewport;

		public:
			explicit ImguiContext(Window& _window, Device& _device);
			~ImguiContext();

            void PrepareDrawData();
			void DrawEditors(void* _vKCommand);
			void PrepareForDrawing();
			void BindPrepareDrawData(std::function<void()> func);
			SceneViewport* GetSceneViewport() { return m_SceneViewport.get(); };
			void DrawViewportComponent(uint32_t width, uint32_t height);
			void RenderSceneToViewport(core::gpu::CommandBuffer* cmd, graphics::Renderer* renderer);

			core::gpu::Image* GetViewportImage();
			void OnResize();
			Impl* GetImpl() { return m_impl.get();};
		};
	}
}