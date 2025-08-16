#ifndef VRAKTAL_RHI_DESCRIPTOR_H
#define VRAKTAL_RHI_DESCRIPTOR_H
#pragma once

#include <memory>

namespace core::rhi::gpu
{
	class Descriptor
	{
		struct Internal;
		std::unique_ptr<Internal> m_Internal;

	public:
		Descriptor();
		Internal& GetInternal() { return *m_Internal; }
	};
}

#endif //VRAKTAL_RHI_DESCRIPTOR_H
