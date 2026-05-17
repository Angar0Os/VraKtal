#pragma once
#include <cstdint>
namespace graphics::assets {
	struct Asset
	{
		uint32_t id;
		bool dirty = true;
	};
}