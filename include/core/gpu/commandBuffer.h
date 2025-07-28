#ifndef VRAKTAL_RHI_COMMAND_BUFFER_H
#define VRAKTAL_RHI_COMMAND_BUFFER_H
#pragma once

#include <memory>
#include <functional>
#include <queue>

#include <core/renderContext.h>

namespace core::gpu::rhi
{
	class CommandBuffer
	{
		struct Internal;
		std::unique_ptr<Internal> m_Internal;
	public:
		explicit CommandBuffer(core::rhi::RenderContext& rCtx);
		Internal& GetInternal() { return *m_Internal; }
	};
}

#endif //VRAKTAL_RHI_COMMAND_BUFFER_H
