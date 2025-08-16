#ifndef VRAKTAL_RHI_MATERIAL_H
#define VRAKTAL_RHI_MATERIAL_H
#pragma once

#include <memory>

namespace graphics::rhi
{
	class Material
	{
		struct Internal;
		std::unique_ptr<Internal> m_Internal;

	public:
		Material();
		Internal& GetInternal() { return *m_Internal; }
	};
}

#endif //VRAKTAL_RHI_MATERIAL_H
