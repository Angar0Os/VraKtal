#ifndef VRAKTAL_LOADERS_TEXTURELOADER_H
#define VRAKTAL_LOADERS_TEXTURELOADER_H

#include <string>

#include <core/gpu/texture.h>

namespace loaders
{
    class Device;

    class TextureLoader
    {
    public:
        static std::unique_ptr<core::gpu::Texture> LoadTexture(const core::gpu::Device& device, std::string& filepath);
    };
}

#endif //VRAKTAL_LOADERS_TEXTURELOADER_H