#ifndef VRAKTAL_GRAPHICS_LOADER_SHADER_LOADER_H
#define VRAKTAL_GRAPHICS_LOADER_SHADER_LOADER_H
#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

namespace graphics::loaders
{
    class ShaderLoader
    {
    public:
        static std::vector<char> ReadShaderFile(const std::string& filepath);
        static VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char>& code);
        static void DestroyShaderModule(VkDevice device, VkShaderModule module);
    };   
}

#endif //VRAKTAL_GRAPHICS_LOADER_SHADER_LOADER_H
