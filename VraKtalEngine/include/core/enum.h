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


    // Note : We will move these probably on something like resources ? or image should be fine tbh.
    struct TextureSet
    {
        void* image = nullptr;      
        void* memory = nullptr;     
        void* view = nullptr;       
        uint32_t mipLevels = 1;

        bool isValid() const { return view != nullptr; }
    };

    struct SamplerHandle
    {
        void* handle = nullptr;     
    };

    struct BufferHandle
    {
        void* handle = nullptr;     
    };

}

#endif //VRAKTAL_CORE_ENUMS_H