#ifndef VRAKTAL_GRAPHICS_PASS_GBUFFER_H
#define VRAKTAL_GRAPHICS_PASS_GBUFFER_H
#pragma once

#include <graphics/pass.h>
#include <graphics/resources/material.h>

#include <core/gpu/buffer.h>
#include <graphics/resources/object/mesh.h>

#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>

namespace graphics
{
	struct GBufferPushConstants {
		glm::mat4 model;
		glm::mat4 prevModel;
	};

	class GBufferPass final : public Pass
	{
	public:
		explicit GBufferPass(Device& device,
			const std::vector<std::unique_ptr<Buffer>>& uniformBuffers);
		~GBufferPass() override = default;

		void Init(Device& device)                                override;
		void UpdateDescriptorSets(uint32_t frameIndex)           override;
		void BindDescriptorSets(CommandBuffer& cmd,
			uint32_t frameIndex)             override;

		void Draw(CommandBuffer& cmd,
			const std::vector<ColorAttachmentDesc>& colorAttachments,
			const DepthAttachmentDesc& depthAttachment, uint32_t currentFrame) override;

		const std::vector<PassAttachment>& GetColorAttachments() const override;
		const PassAttachment* GetDepthAttachment()  const override;
		const PassAttachment* GetVelocityAttachment() const { return &m_colorAttachments[2]; }

		void SetMeshInstances(
			const std::vector<std::pair<resources::Mesh*, glm::mat4>>* instances);

		const DescriptorSetLayout* GetMaterialLayout() const { return m_materialLayout.get(); }

		const Pipeline* GetPipeline() const { return m_pipeline.get(); }
	private:
		Device& m_device;

		const std::vector<std::unique_ptr<Buffer>>& m_uniformBuffers;
		const std::vector<std::pair<resources::Mesh*, glm::mat4>>* m_meshInstances = nullptr;

		std::vector<PassAttachment> m_colorAttachments;
		PassAttachment              m_depthAttachment;
		std::unordered_map<resources::Mesh*, glm::mat4> m_prevTransforms;

		std::unique_ptr<DescriptorSetLayout>         m_materialLayout;
		std::shared_ptr<resources::Material> m_fallbackMaterial;
		std::unordered_map<resources::Mesh*, glm::mat4>	m_prevModelTransforms[Device::s_FRAMES_IN_FLIGHT];

		void CreateAttachments();
		void CreateDescriptorSetLayout();
		void CreateMaterialLayout();
		void CreateFallbackMaterial();
		void CreatePipeline();
		void CreateDescriptorSets();
	};
} 

#endif //VRAKTAL_GRAPHICS_PASS_GBUFFER_H