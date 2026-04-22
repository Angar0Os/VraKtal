#ifndef VRAKTAL_GRAPHICS_IBL_PASS_H
#define VRAKTAL_GRAPHICS_IBL_PASS_H
#pragma once

#include <graphics/pass.h>
#include <core/gpu/accelerationStructure.h>
#include <core/gpu/buffer.h>
#include <vector>
#include <memory>

namespace graphics
{
    class IBLPass final : public Pass
    {
    public:
        IBLPass(core::gpu::Device& device,
            const std::vector<std::unique_ptr<core::gpu::Buffer>>& uniformBuffers);
        ~IBLPass() override = default;

        void Init(core::gpu::Device& device)                    override;
        void UpdateDescriptorSets(uint32_t frameIndex)          override;
        void BindDescriptorSets(core::gpu::CommandBuffer& cmd,
            uint32_t frameIndex)            override;

        void Draw(core::gpu::CommandBuffer& cmd,
            const std::vector<ColorAttachmentDesc>& colorAttachments,
            const DepthAttachmentDesc& depthAttachment,
            uint32_t currentFrame)                        override;

        const std::vector<PassAttachment>& GetColorAttachments() const override;
        const PassAttachment* GetDepthAttachment()  const override;

        void SetGBufferInputs(const std::vector<PassAttachment>& colorAttachments,
            const PassAttachment& depthAttachment);

        void SetTLAS(core::gpu::AccelerationStructure* tlas);

    private:
        void CreateAttachments();
        void CreateDescriptorSetLayout();
        void CreatePipeline();
        void CreateDescriptorSets();

        void LoadEnvironmentMaps();

    private:
        core::gpu::Device& m_device;
        const std::vector<std::unique_ptr<core::gpu::Buffer>>& m_uniformBuffers;

        const std::vector<PassAttachment>* m_gbufferColor = nullptr;
        const PassAttachment* m_gbufferDepth = nullptr;

        core::gpu::AccelerationStructure* m_tlas = nullptr;

        PassAttachment m_envMap;
        PassAttachment m_irradianceMap;

        std::vector<PassAttachment> m_colorAttachments; 
    };
} 

#endif // VRAKTAL_GRAPHICS_IBL_PASS_H