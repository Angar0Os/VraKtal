#ifndef VRAKTAL_LOADERS_TEXTURELOADER_H
#define VRAKTAL_LOADERS_TEXTURELOADER_H

#include <string>

#include <core/gpu/texture.h>
#include "./loaderBase.h"
#include <memory>

namespace loaders
{
    class Device;

    class TextureLoader : public LoaderBase
    {
    public:
        TextureLoader(core::gpu::Device& _device);
        ~TextureLoader();
        static core::gpu::Texture* LoadTexture(const core::gpu::Device& device, const  std::string& filepath);
        std::shared_ptr<void> Load(const std::string& path);

    private:
        core::gpu::Device& m_device;
    };
}

#endif //VRAKTAL_LOADERS_TEXTURELOADER_H