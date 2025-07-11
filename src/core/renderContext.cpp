#include <core/renderContext.h>

#include <GLFW/glfw3.h>

#include <iostream>

#pragma comment(lib, "glfw3.lib")

using namespace vk::core;

RenderContext::RenderContext(const RenderContextDescriptor& descriptor)
{
	if (!glfwInit())
	{
		throw std::runtime_error("Failed to initialize GLFW");
		glfwTerminate();
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, descriptor.resizeable ? GLFW_TRUE : GLFW_FALSE);
	m_window = glfwCreateWindow(descriptor.windowSize.x, descriptor.windowSize.y, descriptor.windowTitle, nullptr, nullptr);

	if (!m_window)
	{
		throw std::runtime_error("Failed to create GLFW window");
		glfwTerminate();
	}
}

RenderContext::~RenderContext()
{
}
