#ifndef VRAKTAL_GRAPHICS_LOADERS_GLTF_LOADER_H
#define VRAKTAL_GRAPHICS_LOADERS_GLTF_LOADER_H
#pragma once

#include "../scene/scene.h"

#include <string>

namespace graphics::loaders
{
    class GltfLoader
    {
    public:
        scene::Scene LoadScene(const std::string& path);
    };
}



#endif //VRAKTAL_GRAPHICS_LOADERS_GLTF_LOADER_H
