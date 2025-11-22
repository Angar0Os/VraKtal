#ifndef VRAKTAL_CORE_ENUMS_H
#define VRAKTAL_CORE_ENUMS_H
#pragma once
#include <cstdint>

namespace core
{
	enum class ImageLayout
	{
		Undefined,
		ShaderReadOnly,
		ColorAttachment,
		DepthStencilAttachment,
		TransferSrc,
		TransferDst,
		Present
	};

	enum class ImageUsage : uint32_t
	{
		None = 0,
		TransferSrc = 1 << 0,
		TransferDst = 1 << 1,
		Sampled = 1 << 2,
		Storage = 1 << 3,
		ColorAttachment = 1 << 4,
		DepthStencilAttachment = 1 << 5,
		TransientAttachment = 1 << 6,
		InputAttachment = 1 << 7
	};

	inline ImageUsage operator|(ImageUsage a, ImageUsage b)
	{
		return static_cast<ImageUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	}

	inline ImageUsage operator&(ImageUsage a, ImageUsage b)
	{
		return static_cast<ImageUsage>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
	}

	enum class ImageTiling
	{
		Optimal,
		Linear
	};

	enum class Filter
	{
		Nearest,
		Linear
	};

	enum class SamplerAddressMode
	{
		Repeat,
		MirroredRepeat,
		ClampToEdge,
		ClampToBorder
	};

	enum class SamplerMipmapMode
	{
		Nearest,
		Linear
	};

	enum class CompareOp
	{
		Never,
		Less,
		Equal,
		LessOrEqual,
		Greater,
		NotEqual,
		GreaterOrEqual,
		Always
	};

	enum class TextureFormat
	{
		Undefined,
		R8_UNorm,
		RG8_UNorm,
		RGB8_UNorm,
		RGBA8_UNorm,
		RGBA8_SRGB,
		R16_Float,
		RG16_Float,
		RGBA16_Float,
		R32_Float,
		RG32_Float,
		RGB32_Float,
		RGBA32_Float,
		Depth16,
		Depth24,
		Depth32F,
		Depth24Stencil8,
		Depth32FStencil8,
		BC1_RGB_UNorm,
		BC3_RGBA_UNorm,
		BC7_RGBA_UNorm,
	};

	enum class BufferUsage : uint32_t
	{
		None = 0,
		TransferSrc = 1 << 0,
		TransferDst = 1 << 1,
		UniformBuffer = 1 << 2,
		StorageBuffer = 1 << 3,
		IndexBuffer = 1 << 4,
		VertexBuffer = 1 << 5,
		IndirectBuffer = 1 << 6,

		ShaderDeviceAddress = 1 << 17,
		AccelerationStructureStorage = 1 << 20,
		AccelerationStructureBuildInput = 1 << 19
	};

	inline BufferUsage operator|(BufferUsage a, BufferUsage b)
	{
		return static_cast<BufferUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	}

	inline BufferUsage operator&(BufferUsage a, BufferUsage b)
	{
		return static_cast<BufferUsage>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
	}

	enum class MemoryProperty : uint32_t
	{
		None = 0,
		DeviceLocal = 1 << 0,
		HostVisible = 1 << 1,
		HostCoherent = 1 << 2,
		HostCached = 1 << 3
	};

	inline MemoryProperty operator|(MemoryProperty a, MemoryProperty b)
	{
		return static_cast<MemoryProperty>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	}

	inline MemoryProperty operator&(MemoryProperty a, MemoryProperty b)
	{
		return static_cast<MemoryProperty>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
	}

	enum class DescriptorType
	{
		UniformBuffer,
		CombinedImageSampler,
		StorageBuffer,
		StorageImage
	};

	enum class ShaderStageFlags : uint32_t
	{
		None = 0,
		Vertex = 1 << 0,
		Fragment = 1 << 1,
		Compute = 1 << 2,
		Geometry = 1 << 3,
		TessellationControl = 1 << 4,
		TessellationEvaluation = 1 << 5,
		AllGraphics = Vertex | Fragment | Geometry | TessellationControl | TessellationEvaluation
	};

	inline ShaderStageFlags operator|(ShaderStageFlags a, ShaderStageFlags b)
	{
		return static_cast<ShaderStageFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	}

	inline ShaderStageFlags operator&(ShaderStageFlags a, ShaderStageFlags b)
	{
		return static_cast<ShaderStageFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
	}

	enum class ShaderStage : uint32_t
	{
		None = 0,
		Vertex = 1 << 0,
		Fragment = 1 << 1,
		Compute = 1 << 2,
		Geometry = 1 << 3,
		TessellationControl = 1 << 4,
		TessellationEvaluation = 1 << 5,
		All = 0x7FFFFFFF
	};

	inline ShaderStage operator|(ShaderStage a, ShaderStage b)
	{
		return static_cast<ShaderStage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	}

	inline ShaderStage operator&(ShaderStage a, ShaderStage b)
	{
		return static_cast<ShaderStage>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
	}

	enum class CommandPoolCreateFlags : uint32_t
	{
		None = 0,
		Transient = 1 << 0,
		ResetCommandBuffer = 1 << 1,
		Protected = 1 << 2
	};

	inline CommandPoolCreateFlags operator|(CommandPoolCreateFlags a, CommandPoolCreateFlags b)
	{
		return static_cast<CommandPoolCreateFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	}

	inline CommandPoolCreateFlags operator&(CommandPoolCreateFlags a, CommandPoolCreateFlags b)
	{
		return static_cast<CommandPoolCreateFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
	}

	enum class CommandBufferLevel
	{
		Primary,
		Secondary
	};

	enum class PresentMode
	{
		Immediate,
		Mailbox,
		Fifo,
		FifoRelaxed
	};

	enum class SampleCount
	{
		e1 = 1,
		e2 = 2,
		e4 = 4,
		e8 = 8,
		e16 = 16,
		e32 = 32,
		e64 = 64
	};

	enum class VertexInputRate
	{
		Vertex,
		Instance
	};

	enum class PrimitiveTopology
	{
		PointList,
		LineList,
		LineStrip,
		TriangleList,
		TriangleStrip,
		TriangleFan
	};

	enum class PolygonMode
	{
		Fill,
		Line,
		Point
	};

	enum class CullMode : uint32_t
	{
		None = 0,
		Front = 1 << 0,
		Back = 1 << 1,
		FrontAndBack = Front | Back
	};

	inline CullMode operator|(CullMode a, CullMode b)
	{
		return static_cast<CullMode>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	}

	inline CullMode operator&(CullMode a, CullMode b)
	{
		return static_cast<CullMode>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
	}

	enum class FrontFace
	{
		CounterClockwise,
		Clockwise
	};

	enum class BlendFactor
	{
		Zero,
		One,
		SrcColor,
		OneMinusSrcColor,
		DstColor,
		OneMinusDstColor,
		SrcAlpha,
		OneMinusSrcAlpha,
		DstAlpha,
		OneMinusDstAlpha
	};

	enum class BlendOp
	{
		Add,
		Subtract,
		ReverseSubtract,
		Min,
		Max
	};

	enum class LogicOp
	{
		Clear,
		And,
		AndReverse,
		Copy,
		AndInverted,
		NoOp,
		Xor,
		Or,
		Nor,
		Equivalent,
		Invert,
		OrReverse,
		CopyInverted,
		OrInverted,
		Nand,
		Set
	};

	enum class ColorComponentFlags : uint32_t
	{
		None = 0,
		R = 1 << 0,
		G = 1 << 1,
		B = 1 << 2,
		A = 1 << 3,
		RGBA = R | G | B | A
	};

	inline ColorComponentFlags operator|(ColorComponentFlags a, ColorComponentFlags b)
	{
		return static_cast<ColorComponentFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	}

	inline ColorComponentFlags operator&(ColorComponentFlags a, ColorComponentFlags b)
	{
		return static_cast<ColorComponentFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
	}

	enum class DynamicState
	{
		Viewport,
		Scissor,
		LineWidth,
		DepthBias,
		BlendConstants,
		DepthBounds,
		StencilCompareMask,
		StencilWriteMask,
		StencilReference
	};
}

#endif // VRAKTAL_CORE_ENUMS_H