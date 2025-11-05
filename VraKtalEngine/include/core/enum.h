#ifndef VRAKTAL_CORE_ENUMS_H
#define VRAKTAL_CORE_ENUMS_H
#pragma once

#include <cstdint>

namespace core
{
    enum class ImageLayout
    {
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
}

#endif //VRAKTAL_CORE_ENUMS_H