#ifndef VRAKTAL_CORE_GPU_DESCRIPTORSET_H
#define VRAKTAL_CORE_GPU_DESCRIPTORSET_H
#pragma once

#include <memory>
#include <vector>
#include <core/enum.h>

namespace core::gpu
{
    class Buffer;      
    class Sampler;     

    class DescriptorSet
    {
    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;

    public:
        DescriptorSet(void* device, std::vector<void*>& sets, size_t frame);
        ~DescriptorSet();

        DescriptorSet& BindBuffer(const Buffer& buffer, size_t offset, size_t range);
        DescriptorSet& BindImage(const Sampler& sampler, const core::TextureSet* texture,
            const core::TextureSet& defaultTexture, ImageLayout layout = ImageLayout::ShaderReadOnly);

        void Update();

        Impl& GetImpl();
    };
}

#endif //VRAKTAL_CORE_VULKAN_DESCRIPTORSET_H