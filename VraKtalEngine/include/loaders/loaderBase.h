#pragma once
#include <string>
#include <memory>

class LoaderBase
{
public:
    LoaderBase() {};
    ~LoaderBase() {};
    virtual std::shared_ptr<void> Load(const std::string& path) = 0;
};


