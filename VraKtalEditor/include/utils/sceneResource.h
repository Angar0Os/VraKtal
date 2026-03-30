#ifndef VRAKTAL_EDITOR_SCENE_RESOURCE_H
#define VRAKTAL_EDITOR_SCENE_RESOURCE_H
#pragma once

#include <graphics/resources/object/object.h>
#include <graphics/resources/property/transform.h>

#include <string>

using namespace graphics::resources::object;
using namespace graphics::resources::property;

namespace utils
{
	struct SceneResource
	{
		std::pair<Object, ObjectType>	object;
		Transform						objectTransform;
		std::string						objectName;
		bool							isInTimeline;
	};
}

#endif //VRAKTAL_EDITOR_SCENE_RESOURCE_H
