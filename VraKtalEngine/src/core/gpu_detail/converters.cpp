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

vk::ImageUsageFlags core::gpu_detail::ToVulkan(core::ImageUsage usage)
{
    vk::ImageUsageFlags flags;

    if ((usage & core::ImageUsage::TransferSrc) != core::ImageUsage::None)
        flags |= vk::ImageUsageFlagBits::eTransferSrc;

    if ((usage & core::ImageUsage::TransferDst) != core::ImageUsage::None)
        flags |= vk::ImageUsageFlagBits::eTransferDst;

    if ((usage & core::ImageUsage::Sampled) != core::ImageUsage::None)
        flags |= vk::ImageUsageFlagBits::eSampled;

    if ((usage & core::ImageUsage::Storage) != core::ImageUsage::None)
        flags |= vk::ImageUsageFlagBits::eStorage;

    if ((usage & core::ImageUsage::ColorAttachment) != core::ImageUsage::None)
        flags |= vk::ImageUsageFlagBits::eColorAttachment;

    if ((usage & core::ImageUsage::DepthStencilAttachment) != core::ImageUsage::None)
        flags |= vk::ImageUsageFlagBits::eDepthStencilAttachment;

    if ((usage & core::ImageUsage::InputAttachment) != core::ImageUsage::None)
        flags |= vk::ImageUsageFlagBits::eInputAttachment;

    return flags;
}

vk::ImageTiling core::gpu_detail::ToVulkan(core::ImageTiling tiling)
{
    switch (tiling)
    {
        case core::ImageTiling::Optimal: return vk::ImageTiling::eOptimal;
        case core::ImageTiling::Linear:  return vk::ImageTiling::eLinear;
        default: return vk::ImageTiling::eOptimal;
    }
}

vk::SampleCountFlagBits core::gpu_detail::ToVulkan(core::SampleCount samples)
{
    switch (samples)
    {
        case core::SampleCount::e1:  return vk::SampleCountFlagBits::e1;
        case core::SampleCount::e2:  return vk::SampleCountFlagBits::e2;
        case core::SampleCount::e4:  return vk::SampleCountFlagBits::e4;
        case core::SampleCount::e8:  return vk::SampleCountFlagBits::e8;
        case core::SampleCount::e16: return vk::SampleCountFlagBits::e16;
        case core::SampleCount::e32: return vk::SampleCountFlagBits::e32;
        case core::SampleCount::e64: return vk::SampleCountFlagBits::e64;
        default: return vk::SampleCountFlagBits::e1;
    }
}

vk::Format core::gpu_detail::ToVulkan(core::TextureFormat format)
{
    switch (format)
    {
        case core::TextureFormat::Undefined:       return vk::Format::eUndefined;
        case core::TextureFormat::R8_UNorm:        return vk::Format::eR8Unorm;
        case core::TextureFormat::RG8_UNorm:       return vk::Format::eR8G8Unorm;
        case core::TextureFormat::RGB8_UNorm:      return vk::Format::eR8G8B8Unorm;
        case core::TextureFormat::RGBA8_UNorm:     return vk::Format::eR8G8B8A8Unorm;
        case core::TextureFormat::RGBA8_SRGB:      return vk::Format::eR8G8B8A8Srgb;
        case core::TextureFormat::R16_Float:       return vk::Format::eR16Sfloat;
        case core::TextureFormat::RG16_Float:      return vk::Format::eR16G16Sfloat;
        case core::TextureFormat::RGBA16_Float:    return vk::Format::eR16G16B16A16Sfloat;
        case core::TextureFormat::R32_Float:       return vk::Format::eR32Sfloat;
        case core::TextureFormat::RG32_Float:      return vk::Format::eR32G32Sfloat;
        case core::TextureFormat::RGB32_Float:     return vk::Format::eR32G32B32Sfloat;
        case core::TextureFormat::RGBA32_Float:    return vk::Format::eR32G32B32A32Sfloat;
        case core::TextureFormat::Depth16:         return vk::Format::eD16Unorm;
        case core::TextureFormat::Depth24:         return vk::Format::eX8D24UnormPack32;
        case core::TextureFormat::Depth32F:        return vk::Format::eD32Sfloat;
        case core::TextureFormat::Depth24Stencil8: return vk::Format::eD24UnormS8Uint;
        case core::TextureFormat::Depth32FStencil8:return vk::Format::eD32SfloatS8Uint;
        case core::TextureFormat::BC1_RGB_UNorm:   return vk::Format::eBc1RgbUnormBlock;
        case core::TextureFormat::BC3_RGBA_UNorm:  return vk::Format::eBc3UnormBlock;
        case core::TextureFormat::BC7_RGBA_UNorm:  return vk::Format::eBc7UnormBlock;
        default: return vk::Format::eR8G8B8A8Srgb;
    }
}

vk::DescriptorType core::gpu_detail::ToVulkan(core::DescriptorType type)
{
    switch (type)
    {
    case core::DescriptorType::UniformBuffer:
        return vk::DescriptorType::eUniformBuffer;
    case core::DescriptorType::CombinedImageSampler:
        return vk::DescriptorType::eCombinedImageSampler;
    case core::DescriptorType::StorageBuffer:
        return vk::DescriptorType::eStorageBuffer;
    case core::DescriptorType::StorageImage:
        return vk::DescriptorType::eStorageImage;
    default:
        throw std::runtime_error("Unknown descriptor type");
    }
}

vk::ShaderStageFlags core::gpu_detail::ToVulkan(core::ShaderStage stage)
{
    vk::ShaderStageFlags flags;

    if (static_cast<int>(stage) & static_cast<int>(core::ShaderStage::Vertex))
        flags |= vk::ShaderStageFlagBits::eVertex;
    if (static_cast<int>(stage) & static_cast<int>(core::ShaderStage::Fragment))
        flags |= vk::ShaderStageFlagBits::eFragment;
    if (static_cast<int>(stage) & static_cast<int>(core::ShaderStage::Compute))
        flags |= vk::ShaderStageFlagBits::eCompute;
    if (static_cast<int>(stage) == static_cast<int>(core::ShaderStage::All))
        flags = vk::ShaderStageFlagBits::eAll;

    return flags;
}

vk::CommandPoolCreateFlags ToVulkanCommandPoolFlags(core::CommandPoolCreateFlags flags)
{
    vk::CommandPoolCreateFlags vkFlags;

    if (static_cast<int>(flags) & static_cast<int>(core::CommandPoolCreateFlags::Transient))
        vkFlags |= vk::CommandPoolCreateFlagBits::eTransient;

    if (static_cast<int>(flags) & static_cast<int>(core::CommandPoolCreateFlags::ResetCommandBuffer))
        vkFlags |= vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

    if (static_cast<int>(flags) & static_cast<int>(core::CommandPoolCreateFlags::Protected))
        vkFlags |= vk::CommandPoolCreateFlagBits::eProtected;

    return vkFlags;
}