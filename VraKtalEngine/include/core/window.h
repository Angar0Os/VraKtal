#ifndef VRAKTAL_CORE_WINDOW_H
#define VRAKTAL_CORE_WINDOW_H
#pragma once

#include <cstdint>
#include <utility>
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#pragma comment(lib, "glfw3.lib")


struct GLFWwindow;

namespace core
{
	class Window
	{
	private:
		GLFWwindow* m_window;
		uint32_t m_width = 0, m_height = 0;
		std::string m_title;
	public:
		Window(uint32_t _width, uint32_t _height, const char* _title, bool _resizable = true);
		~Window();

		void PollEvents();
		bool ShouldClose() const;

		std::pair<uint32_t, uint32_t> Size() const;
		const char* Title() const;

		GLFWwindow* GlfwHandle() const;

		bool framebufferResized = false;
	};
}

#endif //VRAKTAL_CORE_WINDOW_H