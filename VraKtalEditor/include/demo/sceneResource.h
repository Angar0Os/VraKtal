#ifndef VRAKTAL_EDITOR_SCENE_RESOURCE_H
#define VRAKTAL_EDITOR_SCENE_RESOURCE_H
#pragma once

#include <graphics/resources/object/object.h>
#include <graphics/resources/object/camera.h>
#include <graphics/resources/object/light.h>
#include <graphics/resources/object/mesh.h>
#include <graphics/resources/property/transform.h>

#include <string>
#include <variant>

namespace demo
{
    using ObjectVariant = std::variant<graphics::resources::Mesh*, graphics::resources::object::Camera*, graphics::resources::Light*>;

    struct SceneResource
    {
        std::string                                 objectName;
        ObjectVariant                               object;
        graphics::resources::property::Transform    objectTransform;
        bool                                        isInTimeline = false;
    };
}

#endif //VRAKTAL_EDITOR_SCENE_RESOURCE_H