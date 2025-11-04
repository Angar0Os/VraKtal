#ifndef VRAKTAL_CORE_GPU_VULKAN_DESCRIPTORSET_H
#define VRAKTAL_CORE_GPU_VULKAN_DESCRIPTORSET_H
#pragma once

#include <core/gpu/descriptorSet.h>

#include <vulkan/vulkan_raii.hpp>

namespace core::gpu
{
    // Note : I need to move the converters into a utils file.
    vk::ImageLayout ToVulkan(ImageLayout layout)
    {
        switch (layout)
        {
        case ImageLayout::ShaderReadOnly: return vk::ImageLayout::eShaderReadOnlyOptimal;
        case ImageLayout::ColorAttachment: return vk::ImageLayout::eColorAttachmentOptimal;
        case ImageLayout::DepthStencilAttachment: return vk::ImageLayout::eDepthStencilAttachmentOptimal;
        case ImageLayout::TransferSrc: return vk::ImageLayout::eTransferSrcOptimal;
        case ImageLayout::TransferDst: return vk::ImageLayout::eTransferDstOptimal;
        case ImageLayout::Present: return vk::ImageLayout::ePresentSrcKHR;
        default: return vk::ImageLayout::eUndefined;
        }
    }

    vk::Filter ToVulkan(Filter filter)
    {
        switch (filter)
        {
        case Filter::Nearest: return vk::Filter::eNearest;
        case Filter::Linear: return vk::Filter::eLinear;
        default: return vk::Filter::eNearest;
        }
    }

    struct DescriptorSet::Impl
    {
    private:
        vk::raii::Device& device;
        std::vector<vk::raii::DescriptorSet>& descriptorSets;
        size_t currentFrame;
        uint32_t currentBinding = 0;

        std::vector<vk::DescriptorBufferInfo> bufferInfos;
        std::vector<vk::DescriptorImageInfo> imageInfos;
        std::vector<vk::WriteDescriptorSet> writes;

    public:
        explicit Impl(vk::raii::Device& dev, std::vector<vk::raii::DescriptorSet>& sets, size_t frame);
        ~Impl();

        DescriptorSet& BindBuffer(BufferHandle buffer, size_t offset, size_t range);

        DescriptorSet& BindImage(SamplerHandle samplerHandle, const TextureSet* texture, const TextureSet& defaultTexture, ImageLayout layout);
       
        void Update();
        void CreateDescriptorSets();
    };
}

#endif //VRAKTAL_CORE_GPU_VULKAN_DESCRIPTORSET_H
