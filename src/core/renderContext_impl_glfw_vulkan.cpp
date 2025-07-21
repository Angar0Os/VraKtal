#include "renderContext_impl_glfw_vulkan.h"

#include <GLFW/glfw3.h>	
#include <iostream>

#pragma comment(lib, "glfw3.lib")

using namespace rhi;

RenderContext::RenderContext(const RenderContextDescriptor& descriptor)
	: m_Internal(new Internal)
{
	if (!glfwInit())
	{
		throw std::runtime_error("Failed to initialize GLFW");
		glfwTerminate();
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, descriptor.resizeable ? GLFW_TRUE : GLFW_FALSE);
	m_Internal->window = glfwCreateWindow(descriptor.windowSize.x, descriptor.windowSize.y, descriptor.windowTitle, nullptr, nullptr);

	if (!m_Internal->window)
	{
		throw std::runtime_error("Failed to create GLFW window");
		glfwTerminate();
	}
}

RenderContext::~RenderContext()
{
	if (m_Internal->window)
	{
		glfwDestroyWindow(m_Internal->window);
		m_Internal->window = nullptr;
	}
	glfwTerminate();
}
