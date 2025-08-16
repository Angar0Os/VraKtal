#ifndef VRAKTAL_RHI_MESH_H
#define VRAKTAL_RHI_MESH_H
#pragma once

#include <memory>

namespace graphics::rhi
{
	class Mesh
	{
		struct Internal;
		std::unique_ptr<Internal> m_Internal;
	public:
		Mesh();
		Internal& GetInternal() { return *m_Internal; }
	};
}

#endif //VRAKTAL_RHI_MESH_H