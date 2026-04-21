#pragma once
#include <scene/timeline/entities/mesh.h>
#include <imgui/imgui.h>

template<typename T>
struct Inspect
{
    virtual void Draw(T& object) = 0;
};