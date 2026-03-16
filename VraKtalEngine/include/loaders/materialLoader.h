#ifndef VRAKTAL_LOADERS_MATERIALLOADER_H
#define VRAKTAL_LOADERS_MATERIALLOADER_H
#pragma once

#include <graphics/materialInstance.h>
#include <graphics/resources/object/material.h>
#include <core/gpu/device.h>
#include <core/gpu/descriptorSetLayout.h>

#include <memory>
#include <string>

namespace loaders
{
	class MaterialLoader
	{
	public:
		static std::unique_ptr<graphics::resources::MaterialInstance> Load(
			core::gpu::Device& device,
			const graphics::resources::object::Material& material,
			const core::gpu::DescriptorSetLayout* dsLayout);

		static std::unique_ptr<graphics::resources::MaterialInstance> CreateDefault(
			core::gpu::Device& device,
			const core::gpu::DescriptorSetLayout* dsLayout);

	private:
		static std::unique_ptr<core::gpu::Image> UploadTexture(
			core::gpu::Device& device,
			const std::string& filepath,
			bool                isSRGB);

		static std::unique_ptr<core::gpu::Image> CreateFallback1x1(
			core::gpu::Device& device,
			uint8_t r, uint8_t g, uint8_t b, uint8_t a);

		static void BindAndUpdate(
			core::gpu::Device& device,
			graphics::resources::MaterialInstance& instance,
			const core::gpu::DescriptorSetLayout* dsLayout);
	};
}

#endif //VRAKTAL_LOADERS_MATERIALLOADER_H