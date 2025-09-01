#ifndef VRAKTAL_GRAPHICS_SCENE_SCENE_H
#define VRAKTAL_GRAPHICS_SCENE_SCENE_H
#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "../resources/mesh.h"
#include "../resources/material.h"

namespace graphics::scene
{
    struct Node
    {
        glm::mat4 transform;
        int meshIndex;
        std::vector<int> children;
    };
    
    class Scene
    {
    public:
        std::vector<resources::Mesh> meshes;
        std::vector<resources::Material> materials;
        std::vector<Node> nodes;
    };
}



#endif //VRAKTAL_GRAPHICS_SCENE_SCENE_H
