#ifndef VRAKTAL_CORE_GPU_TEXTURE_H
#define VRAKTAL_CORE_GPU_TEXTURE_H
#pragma once

#include <cstdint>
#include <memory>

#include <core/enum.h>

namespace core::gpu
{
    struct TextureCreateInfo
    {
        uint32_t width;
        uint32_t height;
        uint32_t depth = 1;
        TextureFormat format;
        uint32_t mipLevels = 1;
        bool generateMipmaps = false;
    };

    class Texture
    {
    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;

    public:
        Texture(const TextureCreateInfo& info);
        ~Texture();

        bool IsValid() const;
        void* GetImageHandle() const;
        void* GetViewHandle() const;
        void* GetMemoryHandle() const;

        uint32_t GetWidth() const;
        uint32_t GetHeight() const;
        uint32_t GetMipLevels() const;

        void SetData(const void* data, size_t size);
        void GenerateMipmaps();
    };
}

#endif //VRAKTAL_CORE_GPU_TEXTURE_H
