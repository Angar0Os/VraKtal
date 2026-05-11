#ifndef VRAKTAL_CORE_WINDOW_H
#define VRAKTAL_CORE_WINDOW_H
#pragma once

#include <cstdint>
#include <utility>
#include <string>

struct GLFWwindow;

namespace core
{
	using DropCallback = void(*)(GLFWwindow*, int, const char**);

	class Window
	{
	private:
		GLFWwindow* m_window;
		std::string m_title;
		uint32_t m_width = 0, m_height = 0;
	public:

		Window(uint32_t _width, uint32_t _height, const char* _title, bool _resizable = true);
		~Window();

		void PollEvents();
		bool ShouldClose() const;
		void Close();

		void SetDropCallback(DropCallback callback);

		std::pair<uint32_t, uint32_t> Size() const;
		const char* Title() const;

		void SetSize(uint32_t _width, uint32_t _height)
		{
			m_width = _width;
			m_height = _height;
		}

		GLFWwindow* GlfwHandle() const;

		bool framebufferResized = false;
	};
}

#endif //VRAKTAL_CORE_WINDOW_H