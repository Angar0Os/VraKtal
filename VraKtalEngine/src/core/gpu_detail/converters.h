#ifndef VRAKTAL_CORE_GPU_DETAIL_CONVERTERS_H
#define VRAKTAL_CORE_GPU_DETAIL_CONVERTERS_H
#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <core/enum.h>

namespace core::gpu_detail
{
    vk::Filter                  ToVulkan(core::Filter filter);
    vk::SamplerAddressMode      ToVulkan(core::SamplerAddressMode mode);
    vk::SamplerMipmapMode       ToVulkan(core::SamplerMipmapMode mode);
    vk::CompareOp               ToVulkan(core::CompareOp op);
    vk::ImageLayout             ToVulkan(core::ImageLayout layout);
    vk::BufferUsageFlags        ToVulkan(core::BufferUsage usage);
    vk::MemoryPropertyFlags     ToVulkan(core::MemoryProperty properties);
    vk::ImageUsageFlags         ToVulkan(core::ImageUsage usage);
    vk::ImageTiling             ToVulkan(core::ImageTiling tiling);
    vk::SampleCountFlagBits     ToVulkan(core::SampleCount samples);
    vk::Format                  ToVulkan(core::TextureFormat format);
    vk::DescriptorType          ToVulkan(core::DescriptorType type);
    vk::CommandPoolCreateFlags  ToVulkan(core::CommandPoolCreateFlags flags);
    vk::VertexInputRate         ToVulkan(core::VertexInputRate rate);
    vk::DynamicState            ToVulkan(core::DynamicState state);
    vk::ColorComponentFlags     ToVulkan(core::ColorComponentFlags flags);
    vk::LogicOp                 ToVulkan(core::LogicOp op);
    vk::BlendOp                 ToVulkan(core::BlendOp op);
    vk::PresentModeKHR          ToVulkan(core::PresentMode mode);
    vk::PrimitiveTopology       ToVulkan(core::PrimitiveTopology topology);
    vk::PolygonMode             ToVulkan(core::PolygonMode mode);
    vk::CullModeFlags           ToVulkan(core::CullMode mode);
    vk::FrontFace               ToVulkan(core::FrontFace face);
    vk::BlendFactor             ToVulkan(core::BlendFactor factor);
    vk::ShaderStageFlagBits     ToVulkan(core::ShaderStageFlags stage);
    vk::ShaderStageFlags        ToVulkan(core::ShaderStage stages);

    core::TextureFormat         FromVulkan(vk::Format format);
}

#endif //VRAKTAL_CORE_GPU_DETAIL_CONVERTERS_H