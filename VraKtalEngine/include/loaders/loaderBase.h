#pragma once
#include <string>
#include <memory>
#include <iostream>

namespace loaders 
{
    struct LoadOptions
    {
        virtual ~LoadOptions() = default;
    };
    
    class LoaderBase
    {
    public:
        LoaderBase() {};
        virtual ~LoaderBase() = default;
        virtual std::shared_ptr<void> Load(const std::string& path, const LoadOptions* options) {
            std::cout << "THIS CLASS SHOULDN'T EXIST" << std::endl;
            return nullptr;
        };
    };
}