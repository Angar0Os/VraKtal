#pragma once

#include <graphics/resources/material.h>
#include <graphics/assets/material.h>
#include <core/gpu/device.h>
#include <graphics/renderer.h>
#include <core/gpu/descriptorSetLayout.h>

#include <memory>
#include <string>
class RessourceManager;
namespace factory
{
	class MaterialFactory
	{
	public:
		MaterialFactory(core::gpu::Device& device , graphics::Renderer& _renderer);

		graphics::resources::Material Create(const graphics::assets::Material& asset);

		static std::shared_ptr<graphics::resources::Material> CreateMaterialInstance(
			core::gpu::Device& device,
			const graphics::assets::Material& material,
			const core::gpu::DescriptorSetLayout* dsLayout
		);

		static std::shared_ptr<graphics::resources::Material> CreateDefault(
			core::gpu::Device& device,
			const core::gpu::DescriptorSetLayout* dsLayout);

		static std::unique_ptr<core::gpu::Image> UploadHDRTexture(core::gpu::Device&, const std::string&);

		static std::unique_ptr<core::gpu::Image> UploadTexture(
			core::gpu::Device& device,
			const std::string& filepath,
			bool                isSRGB);
		

		static std::unique_ptr<core::gpu::Image> CreateFallback1x1(
			core::gpu::Device& device,
			uint8_t r, uint8_t g, uint8_t b, uint8_t a);

		static void BindAndUpdate(
			core::gpu::Device& device,
			graphics::resources::Material& instance,
			const core::gpu::DescriptorSetLayout* dsLayout);

	private:
		core::gpu::Device& m_device;
		graphics::Renderer& m_renderer;
	};
}