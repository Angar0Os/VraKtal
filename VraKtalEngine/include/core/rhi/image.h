#ifndef VRAKTAL_CORE_RHI_IMAGE_H
#define VRAKTAL_CORE_RHI_IMAGE_H
#pragma once

#include <cstdint>

namespace core::rhi
{
    enum class ImageUsage
    {
        ColorAttachment,
        DepthStencilAttachment,
        Sampled,
        Storage
    };

    class Image
    {
    public:
        virtual ~Image() = default;

        virtual uint32_t Width() const = 0;
        virtual uint32_t Height() const = 0;
    };
}

#endif //VRAKTAL_CORE_RHI_IMAGE_H