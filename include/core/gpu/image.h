#ifndef VRAKTAL_RHI_IMAGE_H
#define VRAKTAL_RHI_IMAGE_H
#pragma once

#include <memory>

namespace core::rhi
{
	class Image
	{
		struct Internal;
		std::unique_ptr<Internal> m_Internal;

	public:
		Image();
		Internal& GetInternal() { return *m_Internal; }
	};
}

#endif //VRAKTAL_RHI_IMAGE_H
