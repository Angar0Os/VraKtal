#ifndef VRAKTAL_CORE_GPU_SAMPLER_H
#define VRAKTAL_CORE_GPU_SAMPLER_H
#pragma once

#include <memory>
#include <core/enum.h>

namespace core::gpu
{
    struct SamplerCreateInfo
    {
        Filter minFilter = Filter::Linear;
        Filter magFilter = Filter::Linear;
        SamplerAddressMode addressModeU = SamplerAddressMode::Repeat;
        SamplerAddressMode addressModeV = SamplerAddressMode::Repeat;
        SamplerAddressMode addressModeW = SamplerAddressMode::Repeat;
        float maxAnisotropy = 1.0f;
        bool enableAnisotropy = false;
    };

    class Sampler
    {
    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;

    public:
        Sampler(const SamplerCreateInfo& info);
        ~Sampler();

        void* GetHandle() const;
    };
}

#endif //VRAKTAL_CORE_GPU_SAMPLER_H