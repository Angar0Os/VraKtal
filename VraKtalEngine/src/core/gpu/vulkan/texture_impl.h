#ifndef VRAKTAL_CORE_GPU_VULKAN_TEXTURE_H
#define VRAKTAL_CORE_GPU_VULKAN_TEXTURE_H
#pragma once

#include <core/gpu/texture.h>
#include <vulkan/vulkan_raii.hpp>
#include <memory>

namespace core::gpu
{
    class Image;
    class Buffer;
    class CommandBuffer;

    struct Texture::Impl
    {
    private:
        Texture& parent;
        vk::raii::Device& device;
        vk::raii::PhysicalDevice& physicalDevice;
        vk::raii::Queue& queue;
        vk::raii::CommandPool& commandPool;

        std::unique_ptr<Image> image;
        uint32_t width;
        uint32_t height;
        uint32_t mipLevels;

        void LoadFromFile(const TextureCreateInfo& info);
        std::unique_ptr<CommandBuffer> BeginSingleTimeCommands();
        void EndSingleTimeCommands(std::unique_ptr<CommandBuffer>& commandBuffer);

        static std::unique_ptr<Image> CreateSolidColorImage(
            vk::raii::Device& device,
            vk::raii::PhysicalDevice& physicalDevice,
            vk::raii::Queue& queue,
            vk::raii::CommandPool& commandPool,
            float r, float g, float b, float a,
            TextureFormat format);

    public:
        explicit Impl(Texture& p, vk::raii::Device& dev, vk::raii::PhysicalDevice& physDev,
            vk::raii::Queue& q, vk::raii::CommandPool& pool,
            const TextureCreateInfo& info);

        explicit Impl(Texture& p, vk::raii::Device& dev, vk::raii::PhysicalDevice& physDev,
            vk::raii::Queue& q, vk::raii::CommandPool& pool,
            float r, float g, float b, float a, TextureFormat format);

        bool LoadTextureIfExists(const std::string& filepath);

        ~Impl();

        Image* GetImage() const;
        uint32_t GetWidth() const { return width; }
        uint32_t GetHeight() const { return height; }
        uint32_t GetMipLevels() const { return mipLevels; }
    };
}

#endif //VRAKTAL_CORE_GPU_VULKAN_TEXTURE_H