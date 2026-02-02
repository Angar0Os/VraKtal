#ifndef VRAKTAL_LOADERS_SHADERLOADER_H
#define VRAKTAL_LOADERS_SHADERLOADER_H
#pragma once

#include <vector>
#include <string>
#include <fstream>

namespace loaders
{
    static std::vector<char> ReadFile(const std::string path)
    {
		std::ifstream file(path, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("Failed to open shader file: " + path);
		}

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
    }
}

#endif //VRAKTAL_LOADERS_SHADERLOADER_H