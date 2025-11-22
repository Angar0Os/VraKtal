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

vk::ShaderStageFlagBits core::gpu_detail::ToVulkan(core::ShaderStageFlags stage)
{
	if ((stage & core::ShaderStageFlags::Vertex) != core::ShaderStageFlags::None)
		return vk::ShaderStageFlagBits::eVertex;
	if ((stage & core::ShaderStageFlags::Fragment) != core::ShaderStageFlags::None)
		return vk::ShaderStageFlagBits::eFragment;
	if ((stage & core::ShaderStageFlags::Compute) != core::ShaderStageFlags::None)
		return vk::ShaderStageFlagBits::eCompute;
	if ((stage & core::ShaderStageFlags::Geometry) != core::ShaderStageFlags::None)
		return vk::ShaderStageFlagBits::eGeometry;
	if ((stage & core::ShaderStageFlags::TessellationControl) != core::ShaderStageFlags::None)
		return vk::ShaderStageFlagBits::eTessellationControl;
	if ((stage & core::ShaderStageFlags::TessellationEvaluation) != core::ShaderStageFlags::None)
		return vk::ShaderStageFlagBits::eTessellationEvaluation;

	return vk::ShaderStageFlagBits::eVertex;
}

vk::ShaderStageFlags core::gpu_detail::ToVulkan(core::ShaderStage stages)
{
	vk::ShaderStageFlags result;

	if ((stages & core::ShaderStage::Vertex) != core::ShaderStage::None)
		result |= vk::ShaderStageFlagBits::eVertex;

	if ((stages & core::ShaderStage::Fragment) != core::ShaderStage::None)
		result |= vk::ShaderStageFlagBits::eFragment;

	if ((stages & core::ShaderStage::Geometry) != core::ShaderStage::None)
		result |= vk::ShaderStageFlagBits::eGeometry;

	if ((stages & core::ShaderStage::Compute) != core::ShaderStage::None)
		result |= vk::ShaderStageFlagBits::eCompute;

	if ((stages & core::ShaderStage::TessellationControl) != core::ShaderStage::None)
		result |= vk::ShaderStageFlagBits::eTessellationControl;

	if ((stages & core::ShaderStage::TessellationEvaluation) != core::ShaderStage::None)
		result |= vk::ShaderStageFlagBits::eTessellationEvaluation;

	return result;
}

vk::CommandPoolCreateFlags core::gpu_detail::ToVulkan(core::CommandPoolCreateFlags flags)
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

vk::PresentModeKHR core::gpu_detail::ToVulkan(core::PresentMode mode)
{
	switch (mode)
	{
	case core::PresentMode::Immediate:   return vk::PresentModeKHR::eImmediate;
	case core::PresentMode::Mailbox:     return vk::PresentModeKHR::eMailbox;
	case core::PresentMode::Fifo:        return vk::PresentModeKHR::eFifo;
	case core::PresentMode::FifoRelaxed: return vk::PresentModeKHR::eFifoRelaxed;
	default: return vk::PresentModeKHR::eFifo;
	}
}

vk::PrimitiveTopology core::gpu_detail::ToVulkan(core::PrimitiveTopology topology)
{
	switch (topology)
	{
	case core::PrimitiveTopology::PointList:     return vk::PrimitiveTopology::ePointList;
	case core::PrimitiveTopology::LineList:      return vk::PrimitiveTopology::eLineList;
	case core::PrimitiveTopology::LineStrip:     return vk::PrimitiveTopology::eLineStrip;
	case core::PrimitiveTopology::TriangleList:  return vk::PrimitiveTopology::eTriangleList;
	case core::PrimitiveTopology::TriangleStrip: return vk::PrimitiveTopology::eTriangleStrip;
	case core::PrimitiveTopology::TriangleFan:   return vk::PrimitiveTopology::eTriangleFan;
	default: return vk::PrimitiveTopology::eTriangleList;
	}
}

vk::PolygonMode core::gpu_detail::ToVulkan(core::PolygonMode mode)
{
	switch (mode)
	{
	case core::PolygonMode::Fill:  return vk::PolygonMode::eFill;
	case core::PolygonMode::Line:  return vk::PolygonMode::eLine;
	case core::PolygonMode::Point: return vk::PolygonMode::ePoint;
	default: return vk::PolygonMode::eFill;
	}
}

vk::CullModeFlags core::gpu_detail::ToVulkan(core::CullMode mode)
{
	vk::CullModeFlags flags;

	if (mode == core::CullMode::None)
		return vk::CullModeFlagBits::eNone;

	if ((mode & core::CullMode::Front) != core::CullMode::None)
		flags |= vk::CullModeFlagBits::eFront;

	if ((mode & core::CullMode::Back) != core::CullMode::None)
		flags |= vk::CullModeFlagBits::eBack;

	return flags;
}

vk::FrontFace core::gpu_detail::ToVulkan(core::FrontFace face)
{
	switch (face)
	{
	case core::FrontFace::CounterClockwise: return vk::FrontFace::eCounterClockwise;
	case core::FrontFace::Clockwise:        return vk::FrontFace::eClockwise;
	default: return vk::FrontFace::eCounterClockwise;
	}
}

vk::BlendFactor core::gpu_detail::ToVulkan(core::BlendFactor factor)
{
	switch (factor)
	{
	case core::BlendFactor::Zero:              return vk::BlendFactor::eZero;
	case core::BlendFactor::One:               return vk::BlendFactor::eOne;
	case core::BlendFactor::SrcColor:          return vk::BlendFactor::eSrcColor;
	case core::BlendFactor::OneMinusSrcColor:  return vk::BlendFactor::eOneMinusSrcColor;
	case core::BlendFactor::DstColor:          return vk::BlendFactor::eDstColor;
	case core::BlendFactor::OneMinusDstColor:  return vk::BlendFactor::eOneMinusDstColor;
	case core::BlendFactor::SrcAlpha:          return vk::BlendFactor::eSrcAlpha;
	case core::BlendFactor::OneMinusSrcAlpha:  return vk::BlendFactor::eOneMinusSrcAlpha;
	case core::BlendFactor::DstAlpha:          return vk::BlendFactor::eDstAlpha;
	case core::BlendFactor::OneMinusDstAlpha:  return vk::BlendFactor::eOneMinusDstAlpha;
	default: return vk::BlendFactor::eZero;
	}
}

vk::BlendOp core::gpu_detail::ToVulkan(core::BlendOp op)
{
	switch (op)
	{
	case core::BlendOp::Add:             return vk::BlendOp::eAdd;
	case core::BlendOp::Subtract:        return vk::BlendOp::eSubtract;
	case core::BlendOp::ReverseSubtract: return vk::BlendOp::eReverseSubtract;
	case core::BlendOp::Min:             return vk::BlendOp::eMin;
	case core::BlendOp::Max:             return vk::BlendOp::eMax;
	default: return vk::BlendOp::eAdd;
	}
}

vk::LogicOp core::gpu_detail::ToVulkan(core::LogicOp op)
{
	switch (op)
	{
	case core::LogicOp::Clear:        return vk::LogicOp::eClear;
	case core::LogicOp::And:          return vk::LogicOp::eAnd;
	case core::LogicOp::AndReverse:   return vk::LogicOp::eAndReverse;
	case core::LogicOp::Copy:         return vk::LogicOp::eCopy;
	case core::LogicOp::AndInverted:  return vk::LogicOp::eAndInverted;
	case core::LogicOp::NoOp:         return vk::LogicOp::eNoOp;
	case core::LogicOp::Xor:          return vk::LogicOp::eXor;
	case core::LogicOp::Or:           return vk::LogicOp::eOr;
	case core::LogicOp::Nor:          return vk::LogicOp::eNor;
	case core::LogicOp::Equivalent:   return vk::LogicOp::eEquivalent;
	case core::LogicOp::Invert:       return vk::LogicOp::eInvert;
	case core::LogicOp::OrReverse:    return vk::LogicOp::eOrReverse;
	case core::LogicOp::CopyInverted: return vk::LogicOp::eCopyInverted;
	case core::LogicOp::OrInverted:   return vk::LogicOp::eOrInverted;
	case core::LogicOp::Nand:         return vk::LogicOp::eNand;
	case core::LogicOp::Set:          return vk::LogicOp::eSet;
	default: return vk::LogicOp::eCopy;
	}
}

vk::ColorComponentFlags core::gpu_detail::ToVulkan(core::ColorComponentFlags flags)
{
	vk::ColorComponentFlags vkFlags;

	if ((flags & core::ColorComponentFlags::R) != core::ColorComponentFlags::None)
		vkFlags |= vk::ColorComponentFlagBits::eR;

	if ((flags & core::ColorComponentFlags::G) != core::ColorComponentFlags::None)
		vkFlags |= vk::ColorComponentFlagBits::eG;

	if ((flags & core::ColorComponentFlags::B) != core::ColorComponentFlags::None)
		vkFlags |= vk::ColorComponentFlagBits::eB;

	if ((flags & core::ColorComponentFlags::A) != core::ColorComponentFlags::None)
		vkFlags |= vk::ColorComponentFlagBits::eA;

	return vkFlags;
}

vk::DynamicState core::gpu_detail::ToVulkan(core::DynamicState state)
{
	switch (state)
	{
	case core::DynamicState::Viewport:           return vk::DynamicState::eViewport;
	case core::DynamicState::Scissor:            return vk::DynamicState::eScissor;
	case core::DynamicState::LineWidth:          return vk::DynamicState::eLineWidth;
	case core::DynamicState::DepthBias:          return vk::DynamicState::eDepthBias;
	case core::DynamicState::BlendConstants:     return vk::DynamicState::eBlendConstants;
	case core::DynamicState::DepthBounds:        return vk::DynamicState::eDepthBounds;
	case core::DynamicState::StencilCompareMask: return vk::DynamicState::eStencilCompareMask;
	case core::DynamicState::StencilWriteMask:   return vk::DynamicState::eStencilWriteMask;
	case core::DynamicState::StencilReference:   return vk::DynamicState::eStencilReference;
	default: return vk::DynamicState::eViewport;
	}
}

vk::VertexInputRate core::gpu_detail::ToVulkan(core::VertexInputRate rate)
{
	switch (rate)
	{
	case core::VertexInputRate::Vertex:   return vk::VertexInputRate::eVertex;
	case core::VertexInputRate::Instance: return vk::VertexInputRate::eInstance;
	default: return vk::VertexInputRate::eVertex;
	}
}

core::TextureFormat core::gpu_detail::FromVulkan(vk::Format format)
{
	switch (format)
	{
	case vk::Format::eB8G8R8A8Srgb:   return core::TextureFormat::RGBA8_SRGB;
	case vk::Format::eR8G8B8A8Srgb:   return core::TextureFormat::RGBA8_SRGB;
	case vk::Format::eR8G8B8A8Unorm:  return core::TextureFormat::RGBA8_UNorm;
	case vk::Format::eD32Sfloat:      return core::TextureFormat::Depth32F;
	case vk::Format::eD24UnormS8Uint: return core::TextureFormat::Depth24Stencil8;
	default: return core::TextureFormat::Undefined;
	}
}