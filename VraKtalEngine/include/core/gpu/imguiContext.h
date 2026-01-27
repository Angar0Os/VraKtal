#pragma once
#include <memory>
#include <iostream>
#include <functional>

namespace core
{
	class Window;
	namespace gpu
	{
        class Device;
        class CommandBuffer;
		class ImguiContext
		{
		private:
			struct Impl;
			std::unique_ptr<Impl> m_impl;
            std::function<void()> m_prepareDrawDataFunc;

		public:
			explicit ImguiContext(Window& _window, Device& _device);
			~ImguiContext();

            void PrepareDrawData();
			void DrawEditors(void* _vKCommand);
			void BindPrepareDrawData(std::function<void()> func);
			//void RenderDrawData(VkCommandBuffer& _commandBuffer, VkImageView _target, VkRenderingInfo info, VkRenderingAttachmentInfo attachement);
		};
	}
}