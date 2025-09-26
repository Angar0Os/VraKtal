#ifndef VRAKTAL_GRAPHICS_RESOURCES_MATERIAL_H
#define VRAKTAL_GRAPHICS_RESOURCES_MATERIAL_H
#pragma once

#include <glm/glm.hpp>

namespace core::graphics::resources
{
    struct Material
    {
        glm::vec4 baseColorFactor{1.0f};

        float matallicFactor{1.0f};
        float roughnessFactor{1.0f};

        int baseColorTexture = -1;
    };
}

#endif //VRAKTAL_GRAPHICS_RESOURCES_MATERIAL_H