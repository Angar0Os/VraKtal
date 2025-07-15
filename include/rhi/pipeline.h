#ifndef VRAKTAL_RHI_PIPELINE_H
#define VRAKTAL_RHI_PIPELINE_H
#pragma once

namespace rhi
{
	class Pipeline
	{
		virtual void Clear() = 0;
		virtual void SetMultisamplingNone() = 0;
		virtual void DisableBlending() = 0;
		virtual void DisableDepthtest() = 0;
		virtual void EnableBlendingAdditive() = 0;
		virtual void EnableBlendingAlphablend() = 0;
	};
}

#endif //VRAKTAL_RHI_PIPELINE_H
