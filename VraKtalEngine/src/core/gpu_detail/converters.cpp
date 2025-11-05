#include "../src/core/gpu_detail/converters.h"

vk::Filter core::gpu_detail::ToVulkan(core::Filter filter)
{
    switch (filter)
    {
        case core::Filter::Nearest: return vk::Filter::eNearest;
        case core::Filter::Linear:  return vk::Filter::eLinear;
        default: return vk::Filter::eLinear;
    }
}

vk::SamplerAddressMode core::gpu_detail::ToVulkan(core::SamplerAddressMode mode)
{
    switch (mode)
    {
        case core::SamplerAddressMode::Repeat:         return vk::SamplerAddressMode::eRepeat;
        case core::SamplerAddressMode::MirroredRepeat: return vk::SamplerAddressMode::eMirroredRepeat;
        case core::SamplerAddressMode::ClampToEdge:    return vk::SamplerAddressMode::eClampToEdge;
        case core::SamplerAddressMode::ClampToBorder:  return vk::SamplerAddressMode::eClampToBorder;
        default: return vk::SamplerAddressMode::eRepeat;
    }
}

vk::SamplerMipmapMode core::gpu_detail::ToVulkan(core::SamplerMipmapMode mode)
{
    switch (mode)
    {
        case core::SamplerMipmapMode::Nearest: return vk::SamplerMipmapMode::eNearest;
        case core::SamplerMipmapMode::Linear:  return vk::SamplerMipmapMode::eLinear;
        default: return vk::SamplerMipmapMode::eLinear;
    }
}

vk::CompareOp core::gpu_detail::ToVulkan(core::CompareOp op)
{
    switch (op)
    {
        case core::CompareOp::Never:          return vk::CompareOp::eNever;
        case core::CompareOp::Less:           return vk::CompareOp::eLess;
        case core::CompareOp::Equal:          return vk::CompareOp::eEqual;
        case core::CompareOp::LessOrEqual:    return vk::CompareOp::eLessOrEqual;
        case core::CompareOp::Greater:        return vk::CompareOp::eGreater;
        case core::CompareOp::NotEqual:       return vk::CompareOp::eNotEqual;
        case core::CompareOp::GreaterOrEqual: return vk::CompareOp::eGreaterOrEqual;
        case core::CompareOp::Always:         return vk::CompareOp::eAlways;
        default: return vk::CompareOp::eAlways;
    }
}

vk::ImageLayout core::gpu_detail::ToVulkan(core::ImageLayout layout)
{
    switch (layout)
    {
        case core::ImageLayout::ShaderReadOnly:         return vk::ImageLayout::eShaderReadOnlyOptimal;
        case core::ImageLayout::ColorAttachment:        return vk::ImageLayout::eColorAttachmentOptimal;
        case core::ImageLayout::DepthStencilAttachment: return vk::ImageLayout::eDepthStencilAttachmentOptimal;
        case core::ImageLayout::TransferSrc:            return vk::ImageLayout::eTransferSrcOptimal;
        case core::ImageLayout::TransferDst:            return vk::ImageLayout::eTransferDstOptimal;
        case core::ImageLayout::Present:                return vk::ImageLayout::ePresentSrcKHR;
        default: return vk::ImageLayout::eUndefined;
    }
}

vk::BufferUsageFlags core::gpu_detail::ToVulkan(core::BufferUsage usage)
{
    vk::BufferUsageFlags flags;

    if ((usage & core::BufferUsage::TransferSrc) != core::BufferUsage::None)
        flags |= vk::BufferUsageFlagBits::eTransferSrc;

    if ((usage & core::BufferUsage::TransferDst) != core::BufferUsage::None)
        flags |= vk::BufferUsageFlagBits::eTransferDst;

    if ((usage & core::BufferUsage::UniformBuffer) != core::BufferUsage::None)
        flags |= vk::BufferUsageFlagBits::eUniformBuffer;

    if ((usage & core::BufferUsage::StorageBuffer) != core::BufferUsage::None)
        flags |= vk::BufferUsageFlagBits::eStorageBuffer;

    if ((usage & core::BufferUsage::IndexBuffer) != core::BufferUsage::None)
        flags |= vk::BufferUsageFlagBits::eIndexBuffer;

    if ((usage & core::BufferUsage::VertexBuffer) != core::BufferUsage::None)
        flags |= vk::BufferUsageFlagBits::eVertexBuffer;

    if ((usage & core::BufferUsage::IndirectBuffer) != core::BufferUsage::None)
        flags |= vk::BufferUsageFlagBits::eIndirectBuffer;

    return flags;
}

vk::MemoryPropertyFlags core::gpu_detail::ToVulkan(core::MemoryProperty properties)
{
    vk::MemoryPropertyFlags flags;

    if ((properties & core::MemoryProperty::DeviceLocal) != core::MemoryProperty::None)
        flags |= vk::MemoryPropertyFlagBits::eDeviceLocal;

    if ((properties & core::MemoryProperty::HostVisible) != core::MemoryProperty::None)
        flags |= vk::MemoryPropertyFlagBits::eHostVisible;

    if ((properties & core::MemoryProperty::HostCoherent) != core::MemoryProperty::None)
        flags |= vk::MemoryPropertyFlagBits::eHostCoherent;

    if ((properties & core::MemoryProperty::HostCached) != core::MemoryProperty::None)
        flags |= vk::MemoryPropertyFlagBits::eHostCached;

    return flags;
}