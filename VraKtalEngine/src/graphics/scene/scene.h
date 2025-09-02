#ifndef VRAKTAL_GRAPHICS_SCENE_SCENE_H
#define VRAKTAL_GRAPHICS_SCENE_SCENE_H
#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>

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

    struct Camera
    {
        glm::mat4 view{ 1.0f };
        glm::mat4 proj{ 1.0f };
    };
    
    class Scene
    {
    public:
        std::vector<resources::Mesh> meshes;
        std::vector<resources::Material> materials;
        std::vector<Node> nodes;

        std::vector<std::string> textures;
        Camera defaultCamera;
    };
}



#endif //VRAKTAL_GRAPHICS_SCENE_SCENE_H
