#ifndef VRAKTAL_CORE_GPU_VULKAN_TEXTURE_H
#define VRAKTAL_CORE_GPU_VULKAN_TEXTURE_H
#pragma once

#include <core/gpu/texture.h>

#include <vulkan/vulkan_raii.hpp>

namespace core::gpu
{
    class Image;

    struct Texture::Impl
    {
        explicit Impl() = default;

        const core::gpu::Image* image   = nullptr;
        vk::raii::Sampler sampler = nullptr;
 
        ~Impl();
    };
}

#endif //VRAKTAL_CORE_GPU_VULKAN_TEXTURE_H