#ifndef VRAKTAL_CORE_GPU_TEXTURE_H
#define VRAKTAL_CORE_GPU_TEXTURE_H
#pragma once

#include <memory>

namespace core::gpu
{
    class Device;
    class Image;

    class Texture
    {
    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;

    public:
        Texture(const core::gpu::Device& _device, const const::core::gpu::Image& _info);

        ~Texture();

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        Texture(Texture&&) noexcept;
        Texture& operator=(Texture&&) noexcept;

        Impl& GetImpl() const;
    };
}

#endif //VRAKTAL_CORE_GPU_TEXTURE_H