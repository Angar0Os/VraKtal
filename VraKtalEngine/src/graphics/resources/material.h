#ifndef VRAKTAL_GRAPHICS_RESOURCES_MATERIAL_H
#define VRAKTAL_GRAPHICS_RESOURCES_MATERIAL_H
#pragma once

#include <glm/glm.hpp>
#include <string>

namespace graphics::resources
{
    class Material
    {
    public:
        glm::vec4 baseColorFactor{1.0f};

        float matallicFactor{1.0f};
        float roughnessFactor{1.0f};

        std::string baseColorTexture;
        std::string matellicRoughnessTexture;
    };
}

#endif //VRAKTAL_GRAPHICS_RESOURCES_MATERIAL_H