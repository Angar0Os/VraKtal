#ifndef VRAKTAL_GRAPHICS_PASS_TAA_H
#define VRAKTAL_GRAPHICS_PASS_TAA_H
#pragma once

#include <array>

#include <graphics/pass.h>
#include <core/gpu/buffer.h>

namespace graphics
{
	class TAAPass final : public Pass
	{
	public:
		explicit TAAPass(Device& device,
			const std::vector<std::unique_ptr<Buffer>>& uniformBuffers);
		~TAAPass() override = default;

		void Init(Device& device)                                          override;
		void UpdateDescriptorSets(uint32_t frameIndex)                     override;
		void BindDescriptorSets(CommandBuffer& cmd, uint32_t frameIndex)   override;

		void Draw(CommandBuffer& cmd,
			const std::vector<ColorAttachmentDesc>& colorAttachments,
			const DepthAttachmentDesc& depthAttachment,
			uint32_t currentFrame) override;

		void SetInputs(const PassAttachment& currentColor,
			const PassAttachment& velocityTex,
			const PassAttachment& depthCurrent,
			const PassAttachment& depthHistory);

		const std::vector<PassAttachment>& GetColorAttachments() const override;
		const PassAttachment* GetDepthAttachment()               const override;

	private:
		Device& m_device;
		const std::vector<std::unique_ptr<Buffer>>& m_uniformBuffers;

		std::array<PassAttachment, 2> m_historyAttachments;
		std::vector<PassAttachment>   m_colorAttachments;

		const PassAttachment* m_currentColor = nullptr;
		const PassAttachment* m_velocityTex = nullptr;
		const PassAttachment* m_depthCurrent = nullptr;
		const PassAttachment* m_depthHistory = nullptr;  

		bool m_firstFrame = true;                        

		void CreateAttachments();
		void CreateDescriptorSetLayout();
		void CreatePipeline();
		void CreateDescriptorSets();
	};
}

#endif // VRAKTAL_GRAPHICS_PASS_TAA_H