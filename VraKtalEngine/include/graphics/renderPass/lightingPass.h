#ifndef VRAKTAL_GRAPHICS_PASS_LIGHTING_H
#define VRAKTAL_GRAPHICS_PASS_LIGHTING_H
#pragma once

#include <graphics/pass.h>

#include <core/gpu/accelerationStructure.h>
#include <core/gpu/buffer.h>
#include <core/gpu/texture.h>
#include <loaders/materialLoader.h>

#include <vector>

namespace graphics
{
	class LightingPass final : public Pass
	{
	public:
		explicit LightingPass(Device& device,
			const std::vector<std::unique_ptr<Buffer>>& uniformBuffers);
		~LightingPass() override = default;

		void Init(Device& device)                                override;
		void UpdateDescriptorSets(uint32_t frameIndex)           override;
		void BindDescriptorSets(CommandBuffer& cmd,
			uint32_t frameIndex)             override;

		void Draw(CommandBuffer& cmd,
			const std::vector<ColorAttachmentDesc>& colorAttachments,
			const DepthAttachmentDesc& depthAttachment) override;

		const std::vector<PassAttachment>& GetColorAttachments() const override;
		const PassAttachment* GetDepthAttachment()  const override;

		void SetGBufferInputs(const std::vector<PassAttachment>& colorAttachments,
			const PassAttachment& depthAttachment);

		void SetTLAS(AccelerationStructure* tlas);

	private:
		Device& m_device;

		const std::vector<std::unique_ptr<Buffer>>& m_uniformBuffers;

		const std::vector<PassAttachment>* m_gbufferColor = nullptr;
		const PassAttachment* m_gbufferDepth = nullptr;
		AccelerationStructure* m_tlas = nullptr;
		PassAttachment m_envMap;

		std::vector<PassAttachment> m_colorAttachments;
		PassAttachment              m_depthAttachment;

		void CreateAttachments();
		void CreateDescriptorSetLayout();
		void CreatePipeline();
		void CreateDescriptorSets();
	};

} 

#endif //VRAKTAL_GRAPHICS_PASS_LIGHTING_H