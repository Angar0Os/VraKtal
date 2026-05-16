#include <core/window.h>
#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#pragma comment(lib, "glfw3.lib")

static void	FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto app = static_cast<core::Window*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
	app->SetSize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
}

core::Window::Window(uint32_t _width, uint32_t _height, const char* _title, bool _resizable)
	: m_title(_title), m_width(_width), m_height(_height)
{
	if (!glfwInit())
	{
		throw std::runtime_error("Failed to initialize GLFW");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, _resizable ? GLFW_TRUE : GLFW_FALSE);

	m_window = glfwCreateWindow((int)m_width, (int)m_height, m_title.c_str(), nullptr, nullptr);

	if (!m_window)
	{
		throw std::runtime_error("Failed to create GLFW window");
	}

	glfwSetWindowUserPointer(m_window, this);
	glfwSetFramebufferSizeCallback(m_window, FramebufferResizeCallback);
}

core::Window::~Window()
{
	if (m_window)
	{
		glfwDestroyWindow(m_window);
		glfwTerminate(); // Note : Maybe we want to not terminate here but on something like engine::cleanup for multiple window support.
	}
}

void core::Window::PollEvents()
{
	glfwPollEvents();
}

void core::Window::SetDropCallback(DropCallback callback)
{
    glfwSetDropCallback(m_window, callback);
}


bool core::Window::ShouldClose() const
{
	return glfwWindowShouldClose(m_window);
}

void core::Window::Close()
{
	if (m_window) {
		glfwSetWindowShouldClose(m_window, GLFW_TRUE);
	}
}

std::pair<uint32_t, uint32_t> core::Window::Size() const
{
	return { m_width, m_height };
}

const char* core::Window::Title() const
{
	return m_title.c_str();
}

GLFWwindow* core::Window::GlfwHandle() const
{
	return m_window;
}