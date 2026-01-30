#ifndef VRAKTAL_LOADERS_SHADERLOADER_H
#define VRAKTAL_LOADERS_SHADERLOADER_H
#pragma once

#include <vector>
#include <string>

namespace loaders
{
    static std::vector<char> ReadFile(const std::string path);
}

#endif //VRAKTAL_LOADERS_SHADERLOADER_H