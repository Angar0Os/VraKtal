#ifndef VRAKTAL_CORE_LOADERS_GLTF_LOADER_H
#define VRAKTAL_CORE_LOADERS_GLTF_LOADER_H
#pragma once

#include <core/graphics/resources/scene.h>
#include <string>

using namespace core::graphics::resources;

namespace core::loaders
{
	Scene LoadScene(const std::string & path);
}

#endif //VRAKTAL_CORE_LOADERS_GLTF_LOADER_H
