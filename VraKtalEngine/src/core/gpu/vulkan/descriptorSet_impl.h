#ifndef VRAKTAL_CORE_GPU_VULKAN_DESCRIPTORSET_H
#define VRAKTAL_CORE_GPU_VULKAN_DESCRIPTORSET_H
#pragma once

#include <core/gpu/descriptorSet.h>
#include <vulkan/vulkan_raii.hpp>

namespace core::gpu
{
    class Buffer;
    class Sampler;
    class Texture;

    struct DescriptorSet::Impl
    {
    private:
        DescriptorSet& parent;
        vk::raii::Device& device;
        std::vector<vk::raii::DescriptorSet>& descriptorSets;
        size_t currentFrame;
        uint32_t currentBinding = 0;

        std::vector<vk::DescriptorBufferInfo> bufferInfos;
        std::vector<vk::DescriptorImageInfo> imageInfos;
        std::vector<vk::WriteDescriptorSet> writes;

    public:
        explicit Impl(DescriptorSet& p, vk::raii::Device& dev,
            std::vector<vk::raii::DescriptorSet>& sets, size_t frame);
        ~Impl();

        DescriptorSet& BindBuffer(const Buffer& buffer, size_t offset, size_t range);
        DescriptorSet& BindImage(const Sampler& sampler, const Texture* texture,
            const Texture& defaultTexture, ImageLayout layout);

        void Update();
    };
}
#endif //VRAKTAL_CORE_GPU_VULKAN_DESCRIPTORSET_H
