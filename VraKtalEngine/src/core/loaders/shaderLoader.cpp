#include <core/loaders/shaderLoader.h>
#include <fstream>
#include <stdexcept>

using namespace core::loaders;

std::vector<char> ShaderLoader::ReadShaderFile(const std::string& filepath)
{
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file " + filepath);
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

VkShaderModule ShaderLoader::CreateShaderModule(VkDevice device, const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module");
    }

    return shaderModule;
}

void ShaderLoader::DestroyShaderModule(VkDevice device, VkShaderModule module)
{
    if (module != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(device, module, nullptr);
    }
}

