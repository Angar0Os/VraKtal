#ifndef VRAKTAL_GRAPHICS_LOADERS_TEXTURE_LOADER_H
#define VRAKTAL_GRAPHICS_LOADERS_TEXTURE_LOADER_H
#pragma once

#include "../resources/texture.h"
#include <vector>
#include <string>

namespace rhi::vulkan { class GpuDeviceVulkan; }
namespace graphics::scene { class Scene; }

namespace graphics::loaders
{
	class TextureLoader
	{
    public:
        TextureLoader(rhi::vulkan::GpuDeviceVulkan& device);
        ~TextureLoader();

        void LoadSceneTextures(const graphics::scene::Scene& scene,
            std::vector<resources::TextureGpu>& texturesOut);

        resources::TextureGpu CreateWhiteFallback();

    private:
        resources::TextureGpu LoadTextureFile(const std::string& path, bool srgb = true);

        rhi::vulkan::GpuDeviceVulkan& m_device;
	};
}

#endif //VRAKTAL_GRAPHICS_LOADERS_TEXTURE_LOADER_H
