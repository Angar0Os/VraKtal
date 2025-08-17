#ifndef VRAKTAL_RHI_RENDER_CONTEXT_H	
#define VRAKTAL_RHI_RENDER_CONTEXT_H	
#pragma once

#include <glm/glm.hpp>
#include <memory>

namespace core::rhi
{
	struct RenderContextDescriptor
	{
		const char* windowTitle = "Vraktal Engine";
		glm::uvec2 windowSize = { 1280, 720 };
		bool resizeable = true;
	};

	class RenderContext
	{
		struct Internal;
		std::unique_ptr<Internal> m_Internal;

	public:
		RenderContext() = default;
		RenderContext(const RenderContextDescriptor& descriptor);
		~RenderContext() noexcept;

		Internal& GetInternal() { return *m_Internal; }
		const Internal& GetInternal() const { return *m_Internal; };

		void BeginFrame();
		bool Present();
	};
}

#endif //VRAKTAL_RHI_RENDER_CONTEXT_H	
