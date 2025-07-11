#ifndef VRAKTAL_CORE_RENDER_CONTEXT_H
#define VRAKTAL_CORE_RENDER_CONTEXT_H
#pragma once

#include "glm/glm.hpp"

struct GLFWwindow;

namespace vk::core
{
	struct RenderContextDescriptor
	{
		const char*	windowTitle = "VrakTal Engine";
		glm::ivec2	windowSize = { 1280, 720 };
		bool		resizeable = true;
	};

	class RenderContext
	{
	private:
		GLFWwindow* m_window = nullptr;

	public:
		RenderContext() = default;
		explicit RenderContext(const RenderContextDescriptor& descriptor);
		~RenderContext() noexcept;

		GLFWwindow* GetWindow() const { return m_window; }
	};
}

#endif // VRAKTAL_CORE_RENDER_CONTEXT_H
