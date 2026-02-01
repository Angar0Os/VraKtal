#include <loaders/textureLoader.h>
#include <core/gpu/image.h>
#include <core/gpu/texture.h>

#include <stb_image.h>

#include <memory>
#include <stdexcept>

std::unique_ptr<core::gpu::Texture> loaders::TextureLoader::LoadTexture(const core::gpu::Device& device, const std::string& filepath)
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

    core::gpu::SImageCreateInfo imageInfo
    {
        .width = static_cast<uint32_t>(width),
        .height = static_cast<uint32_t>(height),
        .mipLevels = 1,
        .arrayLayers = 1,
        .format = core::TextureFormat::RGBA8_SRGB,
        .usage = core::ImageUsage::TransferDst | core::ImageUsage::Sampled
    };

    auto image = std::make_unique<core::gpu::Image>(&device, imageInfo);

    stbi_image_free(pixels);

    auto textureOutput = std::make_unique<core::gpu::Texture>(device, *image);

    return textureOutput;
}
