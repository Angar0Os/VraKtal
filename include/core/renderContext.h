#ifndef VRAKTAL_RHI_RENDER_CONTEXT_H	
#define VRAKTAL_RHI_RENDER_CONTEXT_H	
#pragma once

#include <glm/glm.hpp>
#include <memory>

namespace rhi
{
	struct RenderContextDescriptor
	{
		const char* windowTitle = "Vraktal Engine";
		glm::ivec2 windowSize = { 1280, 720 };
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
	};
}

#endif //VRAKTAL_RHI_RENDER_CONTEXT_H	
