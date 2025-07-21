#ifndef VRAKTAL_CORE_RENDER_CONTEXT_IMPL_GLFW_VULKAN_H
#define VRAKTAL_CORE_RENDER_CONTEXT_IMPL_GLFW_VULKAN_H
#pragma once

#include <core/renderContext.h>

struct GLFWwindow;

struct rhi::RenderContext::Internal
{
	GLFWwindow* window = nullptr;
};

#endif //VRAKTAL_CORE_RENDER_CONTEXT_IMPL_GLFW_VULKAN_H
