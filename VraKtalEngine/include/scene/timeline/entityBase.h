#pragma once
#include "./keyframe.h"
#include <cstdint> 
#include <limits>  

using EntityID = uint32_t;
constexpr EntityID INVALID_ENTITY = std::numeric_limits<EntityID>::max();

struct ComponentBase {
	virtual ~ComponentBase() = default;
	bool bIsActive = true;
};