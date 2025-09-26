#ifndef VRAKTAL_CORE_LOADERS_TEXTURE_LOADER_H
#define VRAKTAL_CORE_LOADERS_TEXTURE_LOADER_H
#pragma once

#include <core/graphics/resources/texture.h>
#include <core/graphics/resources/scene.h>

#include <vector>
#include <string>

namespace core::rhi::vulkan { class GpuDeviceVulkan; }

using namespace core::graphics::resources;
using namespace core::rhi::vulkan;

namespace core::loaders
{
	class TextureLoader
	{
    public:
        TextureLoader(GpuDeviceVulkan& device);
        ~TextureLoader();

        void LoadSceneTextures(const Scene& scene, std::vector<TextureGpu>& texturesOut);

        TextureGpu CreateWhiteFallback();

    private:
        TextureGpu LoadTextureFile(const std::string& path, bool srgb = true);

        GpuDeviceVulkan& m_device;
	};
}

#endif //VRAKTAL_CORE_LOADERS_TEXTURE_LOADER_H
