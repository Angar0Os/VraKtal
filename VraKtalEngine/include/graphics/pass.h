#ifndef VRAKTAL_GRAPHICS_PASS_H
#define VRAKTAL_GRAPHICS_PASS_H
#pragma once

#include <core/gpu/commandBuffer.h>
#include <core/gpu/descriptorSet.h>
#include <core/gpu/descriptorSetLayout.h>
#include <core/gpu/device.h>
#include <core/gpu/image.h>
#include <core/gpu/pipeline.h>
#include <core/gpu/texture.h>

#include <memory>
#include <string>
#include <vector>

using namespace core;
using namespace core::gpu;

namespace graphics
{
	struct PassAttachment
	{
		std::unique_ptr<Image>   image;
		std::unique_ptr<Texture> texture;
	};

	struct ColorAttachmentDesc
	{
		const Image* image = nullptr;
		bool         clear = true;
		float        clearR = 0.0f;
		float        clearG = 0.0f;
		float        clearB = 0.0f;
		float        clearA = 1.0f;
	};

	struct DepthAttachmentDesc
	{
		const Image* image = nullptr;
		bool         clear = true;
		float        clearDepth = 1.0f;
	};

	class Pass
	{
	public:
		explicit Pass(const std::string& name)
			: m_name(name), m_debugEnabled(false)
		{
		}

		virtual ~Pass() = default;

		Pass(const Pass&) = delete;
		Pass& operator=(const Pass&) = delete;


		const std::string& GetName() const { return m_name; }

		bool IsDebugEnabled() const { return m_debugEnabled; }
		void SetDebugEnabled(bool enabled) { m_debugEnabled = enabled; }


		virtual void Init(Device& device) = 0;

		virtual void UpdateDescriptorSets(uint32_t frameIndex) = 0;

		virtual void BindDescriptorSets(CommandBuffer& cmd, uint32_t frameIndex) = 0;

		virtual void Draw(
			CommandBuffer& cmd,
			const std::vector<ColorAttachmentDesc>& colorAttachments,
			const DepthAttachmentDesc& depthAttachment
		) = 0;

		virtual const std::vector<PassAttachment>& GetColorAttachments() const = 0;

		virtual const PassAttachment* GetDepthAttachment() const = 0;

	protected:
		std::string                                  m_name;
		bool                                         m_debugEnabled;

		std::unique_ptr<Pipeline>                    m_pipeline;
		std::vector<std::unique_ptr<DescriptorSetLayout>> m_dsLayouts;
		std::vector<std::unique_ptr<DescriptorSet>>       m_descriptorSets; 
	};

}

#endif //VRAKTAL_GRAPHICS_PASS_H