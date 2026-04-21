#pragma once
#include <memory>
#include <iostream>
#include <functional>
#include <glm/glm.hpp> 


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

			enum EViewportState;

			struct ViewportState
			{
				uint32_t width = 0;
				uint32_t height = 0;
				bool hovered = false;
				bool focused = false;
				bool clicked = false;
			};

			ViewportState m_viewport;

		public:
			explicit ImguiContext(Window& _window, Device& _device);
			~ImguiContext();

            void PrepareDrawData();
			void DrawEditors(void* _vKCommand);
			void PrepareForDrawing();
			void BindPrepareDrawData(std::function<void()> func);
			ViewportState* GetViewportState();
			void DrawViewportComponent(uint32_t width, uint32_t height);
			void RenderSceneToViewport(core::gpu::CommandBuffer* cmd, graphics::Renderer* renderer);

            glm::mat4 GetViewportProjection();

			core::gpu::Image* GetViewportImage();
			void OnResize();
			Impl* GetImpl() { return m_impl.get();};
			

		};
	}
}