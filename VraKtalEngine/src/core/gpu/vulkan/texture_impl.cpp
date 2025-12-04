#include "../src/core/gpu/vulkan/texture_impl.h"
#include "../src/core/gpu/vulkan/device_impl.h"
#include "../src/core/gpu/vulkan/commandPool_impl.h"

#include <core/gpu/image.h>
#include <core/gpu/buffer.h>
#include <core/gpu/commandBuffer.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <fstream>

core::gpu::Texture::Impl::Impl(core::gpu::Texture& _p, const core::gpu::Device* _device, 
                               const core::gpu::CommandPool* _pool,
                               const TextureCreateInfo& _info)
    : parent(_p), device(_device), commandPool(_pool), width(0), height(0), mipLevels(1)
{
    try
    {
        LoadFromFile(_info);
    }
    catch(const std::exception& e)
    {
        std::cerr << "Failed to load texture from file: " << e.what() << std::endl;
        width = 1;
        height = 1;
        image = CreateSolidColorImage(_device, _pool, 1.0f, 0.0f, 1.0f, 1.0f, TextureFormat::RGBA8_SRGB);
    }
}

core::gpu::Texture::Impl::~Impl() = default;

void core::gpu::Texture::Impl::LoadFromFile(const TextureCreateInfo& info)
{
    stbi_set_flip_vertically_on_load(info.flipVertically);

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(info.filepath.c_str(), &texWidth, &texHeight,
                                &texChannels, STBI_rgb_alpha);

    if(!pixels)
    {
        throw std::runtime_error("Failed to load texture image: " + info.filepath);
    }

    width = static_cast<uint32_t>(texWidth);
    height = static_cast<uint32_t>(texHeight);

    if(info.generateMipmaps)
    {
        mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
    }
    else
    {
        mipLevels = 1;
    }

    vk::DeviceSize imageSize = texWidth * texHeight * 4;

    SBufferCreateInfo stagingBufferInfo{};
    stagingBufferInfo.size = imageSize;
    stagingBufferInfo.usage = EBufferUsage::TransferSrc;
    stagingBufferInfo.memoryProperties = EMemoryProperty::HostVisible | EMemoryProperty::HostCoherent;

    Buffer stagingBuffer(device, stagingBufferInfo);
    stagingBuffer.CopyFrom(pixels, imageSize, 0);

    stbi_image_free(pixels);

    SImageCreateInfo imageInfo{};
    imageInfo.width = width;
    imageInfo.height = height;
    imageInfo.mipLevels = mipLevels;
    imageInfo.format = info.format;
    imageInfo.tiling = ImageTiling::Optimal;
    imageInfo.usage = ImageUsage::TransferSrc | ImageUsage::TransferDst | ImageUsage::Sampled;
    imageInfo.memoryProperties = EMemoryProperty::DeviceLocal;
    imageInfo.samples = SampleCount::e1;

    image = std::make_unique<Image>(device, imageInfo);

    auto commandBuffer = BeginSingleTimeCommands();

    image->TransitionLayout(*commandBuffer, ImageLayout::TransferDst, ImageLayout::TransferDst, 1);
    image->CopyFromBuffer(*commandBuffer, stagingBuffer, width, height);

    if(info.generateMipmaps)
    {
        image->GenerateMipmaps(*commandBuffer, width, height, mipLevels);
    }
    else
    {
        image->TransitionLayout(*commandBuffer, ImageLayout::TransferDst,
                                ImageLayout::ShaderReadOnly, 1);
    }

    EndSingleTimeCommands(commandBuffer);

    SImageViewCreateInfo viewInfo{};
    viewInfo.format = info.format;
    viewInfo.baseMipLevel = 0;
    viewInfo.levelCount = mipLevels;
    viewInfo.baseArrayLayer = 0;
    viewInfo.layerCount = 1;
    viewInfo.isDepth = false;

    image->CreateView(viewInfo);
}

std::unique_ptr<core::gpu::CommandBuffer> core::gpu::Texture::Impl::BeginSingleTimeCommands()
{
    SCommandBufferCreateInfo cmdInfo{};
    cmdInfo.device = device;
    cmdInfo.level = ECommandBufferLevel::Primary;
    cmdInfo.count = 1;
    cmdInfo.singleTime = true;

    auto commandBuffer = std::make_unique<CommandBuffer>(device, cmdInfo);
    commandBuffer->Begin(0);

    return commandBuffer;
}

void core::gpu::Texture::Impl::EndSingleTimeCommands(std::unique_ptr<CommandBuffer>& commandBuffer)
{
    commandBuffer->End(0);
    commandBuffer->SubmitAndWait(device);
}

core::gpu::Texture::Texture(const core::gpu::Device* _device, 
                            const core::gpu::CommandPool* _commandPool,
                            const TextureCreateInfo& _info)
{
    m_impl = std::make_unique<Impl>(*this, _device, _commandPool, _info);
}

core::gpu::Texture::Impl::Impl(core::gpu::Texture& _p,
                               const core::gpu::Device* _device,
                               const core::gpu::CommandPool* _commandPool,
                               float _r, float _g, float _b, float _a, TextureFormat _format)

    : parent(_p), device(_device), width(1), height(1), mipLevels(1)
{
    image = CreateSolidColorImage(_device, _commandPool, _r, _g, _b, _a, _format);
}

std::unique_ptr<core::gpu::Image> core::gpu::Texture::Impl::CreateSolidColorImage(
    const core::gpu::Device* device,
    const::core::gpu::CommandPool* commandPool,
    float r, float g, float b, float a,
    TextureFormat format)
{
    const uint32_t width = 1;
    const uint32_t height = 1;

    uint8_t pixelData[4] = {
        static_cast<uint8_t>(std::clamp(r, 0.0f, 1.0f) * 255.0f),
        static_cast<uint8_t>(std::clamp(g, 0.0f, 1.0f) * 255.0f),
        static_cast<uint8_t>(std::clamp(b, 0.0f, 1.0f) * 255.0f),
        static_cast<uint8_t>(std::clamp(a, 0.0f, 1.0f) * 255.0f)
    };

    vk::DeviceSize imageSize = sizeof(pixelData);

    SBufferCreateInfo stagingBufferInfo{
        .size = imageSize,
        .usage = EBufferUsage::TransferSrc,
        .memoryProperties = EMemoryProperty::HostVisible | EMemoryProperty::HostCoherent
    };

    Buffer stagingBuffer(device, stagingBufferInfo);
    stagingBuffer.CopyFrom(pixelData, imageSize, 0);

    SImageCreateInfo imageInfo{
        .width = width,
        .height = height,
        .mipLevels = 1,
        .arrayLayers = 1,
        .format = format,
        .tiling = ImageTiling::Optimal,
        .usage = ImageUsage::TransferDst | ImageUsage::Sampled,
        .memoryProperties = EMemoryProperty::DeviceLocal,
        .samples = SampleCount::e1
    };

    auto image = std::make_unique<Image>(device, imageInfo);

    SCommandBufferCreateInfo cmdInfo{
        .device = device,
        .level = ECommandBufferLevel::Primary,
        .count = 1,
        .singleTime = true
    };
    CommandBuffer cmdBuffer(device, cmdInfo);

    cmdBuffer.Begin(0);
    image->TransitionLayout(cmdBuffer, ImageLayout::Undefined, ImageLayout::TransferDst, 1);
    image->CopyFromBuffer(cmdBuffer, stagingBuffer, width, height);
    image->TransitionLayout(cmdBuffer, ImageLayout::TransferDst, ImageLayout::ShaderReadOnly, 1);
    cmdBuffer.End(0);
    cmdBuffer.SubmitAndWait(device);

    SImageViewCreateInfo viewInfo{
        .format = format,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
        .isDepth = false
    };
    image->CreateView(viewInfo);

    return image;
}

bool core::gpu::Texture::Impl::LoadTextureIfExists(const core::gpu::Device* device, const std::string& filepath)
{
    std::ifstream file(filepath);
    if(!file.good())
    {
        std::cerr << "Texture file not found: " << filepath << ", using default color" << std::endl;
        return false;
    }

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight,
                                &texChannels, STBI_rgb_alpha);

    if(!pixels)
    {
        std::cerr << "Failed to load texture image: " << filepath << std::endl;
        return false;
    }

    width = static_cast<uint32_t>(texWidth);
    height = static_cast<uint32_t>(texHeight);

    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    vk::DeviceSize imageSize = texWidth * texHeight * 4;

    SBufferCreateInfo stagingBufferInfo{
        .size = imageSize,
        .usage = EBufferUsage::TransferSrc,
        .memoryProperties = EMemoryProperty::HostVisible | EMemoryProperty::HostCoherent
    };

    Buffer stagingBuffer(device, stagingBufferInfo);

    stagingBuffer.CopyFrom(pixels, imageSize, 0);

    stbi_image_free(pixels);

    SImageCreateInfo imageInfo{
        .width = width,
        .height = height,
        .mipLevels = mipLevels,
        .arrayLayers = 1,
        .format = TextureFormat::RGBA8_SRGB,
        .tiling = ImageTiling::Optimal,
        .usage = ImageUsage::TransferSrc | ImageUsage::TransferDst | ImageUsage::Sampled,
        .memoryProperties = EMemoryProperty::DeviceLocal,
        .samples = SampleCount::e1
    };

    image = std::make_unique<Image>(device, imageInfo);

    SCommandBufferCreateInfo cmdInfo{
        .device = device,
        .level = ECommandBufferLevel::Primary,
        .count = 1,
        .singleTime = true
    };
    CommandBuffer cmdBuffer(device, cmdInfo);

    cmdBuffer.Begin(0);

    image->TransitionLayout(cmdBuffer, ImageLayout::Undefined,
                            ImageLayout::TransferDst, mipLevels);

    image->CopyFromBuffer(cmdBuffer, stagingBuffer, width, height);

    image->GenerateMipmaps(cmdBuffer, width, height, mipLevels);

    cmdBuffer.End(0);
    cmdBuffer.SubmitAndWait(device);

    SImageViewCreateInfo viewInfo{
        .format = TextureFormat::RGBA8_SRGB,
        .baseMipLevel = 0,
        .levelCount = mipLevels,
        .baseArrayLayer = 0,
        .layerCount = 1,
        .isDepth = false
    };
    image->CreateView(viewInfo);

    return true;
}

core::gpu::Texture::~Texture() = default;

core::gpu::Texture::Texture(Texture&&) noexcept = default;
core::gpu::Texture& core::gpu::Texture::operator=(Texture&&) noexcept = default;

core::gpu::Texture::Impl& core::gpu::Texture::GetImpl() const
{
    return *m_impl;
}

core::gpu::Texture::Texture(const core::gpu::Device* device, 
                            const core::gpu::CommandPool* pool,
                            float r, float g, float b, float a, TextureFormat format)
{
    m_impl = std::make_unique<Impl>(*this, device, pool, r, g, b, a, format);
}

bool core::gpu::Texture::LoadTextureIfExists(const core::gpu::Device* device, const std::string& filepath)
{
    return m_impl->LoadTextureIfExists(device, filepath);
}