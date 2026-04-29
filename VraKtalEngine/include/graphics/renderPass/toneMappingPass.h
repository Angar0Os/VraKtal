#ifndef VRAKTAL_GRAPHICS_TONEMAPPING_PASS_H
#define VRAKTAL_GRAPHICS_TONEMAPPING_PASS_H
#pragma once

#include <graphics/pass.h>
#include <core/gpu/buffer.h>

namespace graphics
{
    class ToneMappingPass final : public Pass
    {
    public:
        explicit ToneMappingPass(Device& device);
        ~ToneMappingPass() override = default;

        void Init(Device& device)                                        override;
        void UpdateDescriptorSets(uint32_t frameIndex)                   override;
        void BindDescriptorSets(CommandBuffer& cmd, uint32_t frameIndex) override;

        void Draw(CommandBuffer& cmd,
            const std::vector<ColorAttachmentDesc>& colorAttachments,
            const DepthAttachmentDesc& depthAttachment,
            uint32_t currentFrame) override;

        void SetInput(const PassAttachment& input);

        const std::vector<PassAttachment>& GetColorAttachments() const override;
        const PassAttachment* GetDepthAttachment()  const override;

    private:
        Device& m_device;
        const PassAttachment* m_input = nullptr;

        std::vector<PassAttachment> m_colorAttachments;

        void CreateAttachments();
        void CreateDescriptorSetLayout();
        void CreatePipeline();
        void CreateDescriptorSets();
    };
}

#endif //VRAKTAL_GRAPHICS_TONEMAPPING_PASS_H