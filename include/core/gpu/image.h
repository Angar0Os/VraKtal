#ifndef VRAKTAL_RHI_CORE_GPU_IMAGE_H
#define VRAKTAL_RHI_CORE_GPU_IMAGE_H
#pragma once

#include <cstdint>

namespace rhi::core::gpu
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

#endif //VRAKTAL_RHI_CORE_GPU_IMAGE_H