#ifndef VRAKTAL_CORE_LOADER_SHADER_LOADER_H
#define VRAKTAL_CORE_LOADER_SHADER_LOADER_H
#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

namespace core::loaders
{
    class ShaderLoader
    {
    public:
        static std::vector<char> ReadShaderFile(const std::string& filepath);
        static VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char>& code);
        static void DestroyShaderModule(VkDevice device, VkShaderModule module);
    };   
}

#endif //VRAKTAL_CORE_LOADER_SHADER_LOADER_H
