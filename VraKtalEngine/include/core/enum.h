#ifndef VRAKTAL_CORE_ENUMS_H
#define VRAKTAL_CORE_ENUMS_H
#pragma once

#include <cstdint>

namespace core
{
    enum class ImageUsage : uint32_t
    {
        None = 0,
        TransferSrc = 1 << 0,
        TransferDst = 1 << 1,
        Sampled = 1 << 2,
        Storage = 1 << 3,
        ColorAttachment = 1 << 4,
        DepthStencilAttachment = 1 << 5,
        InputAttachment = 1 << 6
    };

    inline ImageUsage operator|(ImageUsage a, ImageUsage b)
    {
        return static_cast<ImageUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    inline ImageUsage operator&(ImageUsage a, ImageUsage b)
    {
        return static_cast<ImageUsage>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
    }

    inline bool operator!=(ImageUsage a, ImageUsage b)
    {
        return static_cast<uint32_t>(a) != static_cast<uint32_t>(b);
    }

    enum class ImageTiling
    {
        Optimal,
        Linear
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
        IndirectBuffer = 1 << 6
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

    enum class CommandBufferLevel
    {
        Primary,
        Secondary
    };
}

#endif //VRAKTAL_CORE_ENUMS_H