#ifndef VRAKTAL_CORE_GPU_VULKAN_TEXTURE_H
#define VRAKTAL_CORE_GPU_VULKAN_TEXTURE_H
#pragma once

#include <core/gpu/texture.h>
#include <memory>

namespace core::gpu
{
    class Image;
    class Buffer;
    class CommandBuffer;

    struct Texture::Impl
    {
        Texture& parent;
        const core::gpu::Device* device;
        const core::gpu::CommandPool* commandPool;

        std::unique_ptr<Image> image;
        uint32_t width;
        uint32_t height;
        uint32_t mipLevels;

        void LoadFromFile(const TextureCreateInfo& info);
        std::unique_ptr<CommandBuffer> BeginSingleTimeCommands();
        void EndSingleTimeCommands(std::unique_ptr<CommandBuffer>& commandBuffer);

        static std::unique_ptr<Image> CreateSolidColorImage(
            const core::gpu::Device* device, 
            const core::gpu::CommandPool* commandPool,
            float r, float g, float b, float a,
            TextureFormat format);

        explicit Impl(Texture& p, const core::gpu::Device* device, const core::gpu::CommandPool* pool,
                      const TextureCreateInfo& info);

        explicit Impl(Texture& p, const core::gpu::Device* device, const core::gpu::CommandPool* pool,
                      float r, float g, float b, float a, TextureFormat format);

        bool LoadTextureIfExists(const core::gpu::Device* device, const std::string& filepath);

        ~Impl();
    };
}

#endif //VRAKTAL_CORE_GPU_VULKAN_TEXTURE_H