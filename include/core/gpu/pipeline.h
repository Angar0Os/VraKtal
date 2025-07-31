#ifndef VRAKTAL_RHI_PIPELINE_H
#define VRAKTAL_RHI_PIPELINE_H
#pragma once

#include <memory>

namespace core::rhi::gpu
{
	class Pipeline
	{
		struct Internal;
		std::unique_ptr<Internal> m_Internal;

	public:
		~Pipeline();
		Internal& GetInternal() { return *m_Internal; }
	};
}

#endif //VRAKTAL_RHI_PIPELINE_H
