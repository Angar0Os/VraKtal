#pragma once
#include "./keyframe.h"
#include <cstdint> 
#include <limits>  

using EntityID = uint32_t;
constexpr EntityID INVALID_ENTITY = std::numeric_limits<EntityID>::max();

struct EntityBase {
	EntityID ID = INVALID_ENTITY;
	bool bIsActive = true;
};