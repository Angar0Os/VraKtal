#ifndef VRAKTAL_GRAPHICS_PASS_GBUFFER_H
#define VRAKTAL_GRAPHICS_PASS_GBUFFER_H
#pragma once

#include <graphics/pass.h>
#include <graphics/renderer.h>  

#include <core/gpu/buffer.h>

#include <graphics/resources/object/mesh.h>

#include <glm/glm.hpp>

#include <vector>

namespace graphics
{
	class GBufferPass final : public Pass
	{
	public:
		explicit GBufferPass(Device& device,
			const std::vector<std::unique_ptr<Buffer>>& uniformBuffers);
		~GBufferPass() override = default;

		void Init(Device& device)                                  override;
		void UpdateDescriptorSets(uint32_t frameIndex)             override;
		void BindDescriptorSets(CommandBuffer& cmd,
			uint32_t frameIndex)               override;

		void Draw(CommandBuffer& cmd,
			const std::vector<ColorAttachmentDesc>& colorAttachments,
			const DepthAttachmentDesc& depthAttachment) override;

		const std::vector<PassAttachment>& GetColorAttachments() const override;
		const PassAttachment* GetDepthAttachment()  const override;

		void SetMeshInstances(
			const std::vector<std::pair<resources::Mesh*, glm::mat4>>* instances);

	private:
		Device& m_device;

		const std::vector<std::unique_ptr<Buffer>>& m_uniformBuffers;

		const std::vector<std::pair<resources::Mesh*, glm::mat4>>* m_meshInstances = nullptr;

		std::vector<PassAttachment> m_colorAttachments; 
		PassAttachment              m_depthAttachment;

		void CreateAttachments();
		void CreateDescriptorSetLayout();
		void CreatePipeline();
		void CreateDescriptorSets();
	};

} 

#endif //VRAKTAL_GRAPHICS_PASS_GBUFFER_H