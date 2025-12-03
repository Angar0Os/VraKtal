#ifndef VRAKTAL_CORE_GPU_TEXTURE_H
#define VRAKTAL_CORE_GPU_TEXTURE_H
#pragma once

#include <memory>
#include <string>
#include <cstdint>

#include <core/enum.h>

namespace core::gpu
{
    class CommandPool;
    class Device;
    class Image;
    class Sampler;

    struct TextureCreateInfo
    {
        std::string filepath;
        TextureFormat format = TextureFormat::RGBA8_SRGB;
        Filter minFilter = Filter::Linear;
        Filter magFilter = Filter::Linear;
        SamplerAddressMode addressMode = SamplerAddressMode::Repeat;
        bool generateMipmaps = true;
        bool flipVertically = false;
    };

    class Texture
    {
    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;

    public:
        Texture(const core::gpu::Device* _device,
                const core::gpu::CommandPool* _pool, 
                const TextureCreateInfo& _info);

        Texture(const core::gpu::Device* _device,
                const core::gpu::CommandPool* _pool,
                float _r, float _g, float _b, float _a,
                TextureFormat _format = TextureFormat::RGBA8_SRGB);

        bool LoadTextureIfExists(const core::gpu::Device* device, const std::string& _filepath);

        ~Texture();

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        Texture(Texture&&) noexcept;
        Texture& operator=(Texture&&) noexcept;

        Image* GetImage() const;
        void* GetImageView() const;
        uint32_t GetWidth() const;
        uint32_t GetHeight() const;
        uint32_t GetMipLevels() const;

        bool IsValid() const { return GetImageView() != nullptr; }

        Impl& GetImpl();
        const Impl& GetImpl() const;
    };
}

#endif //VRAKTAL_CORE_GPU_TEXTURE_H