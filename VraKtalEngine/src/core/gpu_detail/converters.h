#ifndef VRAKTAL_CORE_GPU_DETAIL_CONVERTERS_H
#define VRAKTAL_CORE_GPU_DETAIL_CONVERTERS_H
#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <core/enum.h>

namespace core::gpu_detail
{
    vk::Filter              ToVulkan(core::Filter filter);
    vk::SamplerAddressMode  ToVulkan(core::SamplerAddressMode mode);
    vk::SamplerMipmapMode   ToVulkan(core::SamplerMipmapMode mode);
    vk::CompareOp           ToVulkan(core::CompareOp op);
    vk::ImageLayout         ToVulkan(core::ImageLayout layout);
    vk::BufferUsageFlags    ToVulkan(core::BufferUsage usage);
    vk::MemoryPropertyFlags ToVulkan(core::MemoryProperty properties);
    vk::ImageUsageFlags     ToVulkan(core::ImageUsage usage);
    vk::ImageTiling         ToVulkan(core::ImageTiling tiling);
    vk::SampleCountFlagBits ToVulkan(core::SampleCount samples);
    vk::Format              ToVulkan(core::TextureFormat format);
}

#endif //VRAKTAL_CORE_GPU_DETAIL_CONVERTERS_H