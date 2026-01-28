#include "../src/core/gpu/vulkan/texture_impl.h"
#include "../src/core/gpu/vulkan/device_impl.h"

#include <core/gpu/image.h>


core::gpu::Texture::Texture(const core::gpu::Device& _device, const core::gpu::Image& _image)
    : m_impl(std::make_unique<Impl>())
{ 
    vk::PhysicalDeviceProperties properties = _device.GetImpl().physicalDevice.getProperties();

    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.anisotropyEnable = vk::True;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.compareEnable = vk::False;
    samplerInfo.compareOp = vk::CompareOp::eAlways;

    m_impl->image = &_image;
    m_impl->sampler = vk::raii::Sampler(_device.GetImpl().device, samplerInfo);
}
