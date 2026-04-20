#include <loaders/textureLoader.h>
#include <core/gpu/image.h>
#include <core/gpu/texture.h>

#include <stb_image.h>

#include <memory>
#include <stdexcept>

loaders::TextureLoader::TextureLoader(core::gpu::Device& _device) : m_device(_device)
{
}

loaders::TextureLoader::~TextureLoader()
{
}

core::gpu::Texture* loaders::TextureLoader::LoadTexture(const core::gpu::Device& device, const std::string& filepath)
{
    int width, height, channels;

    stbi_uc* pixels = stbi_load(
        filepath.c_str(),
        &width,
        &height,
        &channels,
        STBI_rgb_alpha
    );

    if(!pixels)
    {
        throw std::runtime_error("Failed to load texture" + filepath);
    }

    core::gpu::SImageCreateInfo imageInfo{};

    imageInfo.width = static_cast<uint32_t>(width);
    imageInfo.height = static_cast<uint32_t>(height);
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = core::TextureFormat::RGBA8_SRGB;
    imageInfo.usage = core::ImageUsage::TransferDst | core::ImageUsage::Sampled;

    auto image = std::make_unique<core::gpu::Image>(&device, imageInfo);

    stbi_image_free(pixels);

    auto textureOutput = new core::gpu::Texture(device, *image);

    return textureOutput;
}

std::shared_ptr<void> loaders::TextureLoader::Load(const std::string& path)
{
    core::gpu::Texture* raw = LoadTexture(m_device, path);
    if (!raw)
        return nullptr;

    std::shared_ptr<core::gpu::Texture> tex(raw);
    return tex;
}
