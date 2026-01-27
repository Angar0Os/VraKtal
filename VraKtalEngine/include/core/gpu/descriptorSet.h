#ifndef VRAKTAL_CORE_GPU_DESCRIPTORSET_H
#define VRAKTAL_CORE_GPU_DESCRIPTORSET_H
#pragma once

#include <memory>

#include <core/enum.h>

namespace core::gpu
{
    class AccelerationStructure;
    class Buffer;     
    class Device;
    class Sampler;     
    class Texture;

    class DescriptorSet
    {
    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;

    public:
        DescriptorSet(const core::gpu::Device* device, void* setsVector, size_t frame);
        ~DescriptorSet();

        DescriptorSet& BindBuffer(const Buffer& buffer, size_t offset, size_t range);
        DescriptorSet& BindImage(const Sampler& sampler, const Texture* texture,
            const Texture& defaultTexture, ImageLayout layout = ImageLayout::ShaderReadOnly);

        template<typename T>
        void Bind(uint32_t binding, const T& input);
        
        void Update();

        Impl& GetImpl() const;
    };
}

#endif //VRAKTAL_CORE_VULKAN_DESCRIPTORSET_H