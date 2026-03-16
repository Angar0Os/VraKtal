#include <loaders/materialLoader.h>

#include <core/gpu/buffer.h>
#include <core/gpu/commandBuffer.h>
#include <core/enum.h>

#include <stb_image.h>

#include <stdexcept>
#include <iostream>

using namespace core;
using namespace core::gpu;
using namespace graphics::resources;

std::unique_ptr<Image> loaders::MaterialLoader::UploadTexture(
	Device& device,
	const std::string& filepath,
	bool               isSRGB)
{
	int width, height, channels;
	stbi_uc* pixels = stbi_load(filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

	if (!pixels)
	{
		std::cerr << "MaterialLoader: failed to load " << filepath << ", using fallback\n";
		return nullptr;
	}

	size_t imageSize = static_cast<size_t>(width) * height * 4;

	SBufferCreateInfo stagingInfo{
		.size = imageSize,
		.usage = EBufferUsage::TransferSrc,
		.memoryProperties = EMemoryProperty::HostVisible | EMemoryProperty::HostCoherent
	};
	auto staging = std::make_unique<Buffer>(&device, stagingInfo);
	staging->CopyFrom(pixels, imageSize);
	stbi_image_free(pixels);

	SImageCreateInfo imageInfo{
		.width = static_cast<uint32_t>(width),
		.height = static_cast<uint32_t>(height),
		.mipLevels = 1,
		.format = isSRGB ? TextureFormat::RGBA8_SRGB : TextureFormat::RGBA8_UNorm,
		.tiling = ImageTiling::Optimal,
		.usage = ImageUsage::TransferDst | ImageUsage::Sampled,
		.memoryProperties = EMemoryProperty::DeviceLocal,
		.samples = SampleCount::e1
	};
	auto image = std::make_unique<Image>(&device, imageInfo);

	SCommandBufferCreateInfo cmdInfo{
		.device = &device,
		.level = ECommandBufferLevel::Primary,
		.count = 1,
		.singleTime = true
	};
	CommandBuffer cmd(&device, cmdInfo);
	cmd.Begin(0);

	cmd.TransitionImageLayout(image.get(), ImageLayout::Undefined, ImageLayout::TransferDst, false);
	cmd.CopyBufferToImage(staging.get(), image.get(), width, height);
	cmd.TransitionImageLayout(image.get(), ImageLayout::TransferDst, ImageLayout::ShaderReadOnly, false);

	cmd.End(0);
	cmd.SubmitImmediate(&device);

	return image;
}

std::unique_ptr<Image> loaders::MaterialLoader::CreateFallback1x1(
	Device& device,
	uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	uint8_t pixels[4] = { r, g, b, a };

	SBufferCreateInfo stagingInfo{
		.size = 4,
		.usage = EBufferUsage::TransferSrc,
		.memoryProperties = EMemoryProperty::HostVisible | EMemoryProperty::HostCoherent
	};
	auto staging = std::make_unique<Buffer>(&device, stagingInfo);
	staging->CopyFrom(pixels, 4);

	SImageCreateInfo imageInfo{
		.width = 1,
		.height = 1,
		.mipLevels = 1,
		.format = TextureFormat::RGBA8_UNorm,
		.tiling = ImageTiling::Optimal,
		.usage = ImageUsage::TransferDst | ImageUsage::Sampled,
		.memoryProperties = EMemoryProperty::DeviceLocal,
		.samples = SampleCount::e1
	};
	auto image = std::make_unique<Image>(&device, imageInfo);

	SCommandBufferCreateInfo cmdInfo{
		.device = &device,
		.level = ECommandBufferLevel::Primary,
		.count = 1,
		.singleTime = true
	};
	CommandBuffer cmd(&device, cmdInfo);
	cmd.Begin(0);
	cmd.TransitionImageLayout(image.get(), ImageLayout::Undefined, ImageLayout::TransferDst, false);
	cmd.CopyBufferToImage(staging.get(), image.get(), 1, 1);
	cmd.TransitionImageLayout(image.get(), ImageLayout::TransferDst, ImageLayout::ShaderReadOnly, false);
	cmd.End(0);
	cmd.SubmitImmediate(&device);

	return image;
}

void loaders::MaterialLoader::BindAndUpdate(
	Device& device,
	MaterialInstance& instance,
	const core::gpu::DescriptorSetLayout* dsLayout)
{
	instance.descriptorSet = std::make_unique<DescriptorSet>(&device, const_cast<DescriptorSetLayout*>(dsLayout));

	instance.descriptorSet->Bind(1, *instance.albedoTexture);
	instance.descriptorSet->Bind(2, *instance.normalTexture);
	instance.descriptorSet->Bind(3, *instance.roughnessMetalTexture);
	instance.descriptorSet->Update(device);
}

std::unique_ptr<MaterialInstance> loaders::MaterialLoader::Load(
	Device& device,
	const graphics::resources::object::Material& material,
	const core::gpu::DescriptorSetLayout* dsLayout)
{
	auto instance = std::make_unique<MaterialInstance>();
	instance->name = material.name;
	instance->albedoColor = material.albedo;
	instance->metalness = material.metallic;
	instance->roughnessValue = material.roughness;

	if (material.useAlbedoTexture && !material.albedoTexture.empty())
	{
		instance->albedoImage = UploadTexture(device, material.albedoTexture, true);
	}
	if (!instance->albedoImage)
	{
		uint8_t r = static_cast<uint8_t>(material.albedo.r * 255);
		uint8_t g = static_cast<uint8_t>(material.albedo.g * 255);
		uint8_t b = static_cast<uint8_t>(material.albedo.b * 255);
		instance->albedoImage = CreateFallback1x1(device, r, g, b, 255);
	}
	else
	{
		instance->hasAlbedoTexture = true;
	}
	instance->albedoTexture = std::make_unique<core::gpu::Texture>(device, *instance->albedoImage);

	if (material.useNormalTexture && !material.normalTexture.empty())
	{
		instance->normalImage = UploadTexture(device, material.normalTexture, false);
	}
	if (!instance->normalImage)
	{
		instance->normalImage = CreateFallback1x1(device, 128, 128, 255, 255);
	}
	else
	{
		instance->hasNormalTexture = true;
	}
	instance->normalTexture = std::make_unique<core::gpu::Texture>(device, *instance->normalImage);

	bool hasRM = (material.useRoughnessTexture && !material.roughnessTexture.empty())
		|| (material.useMetallicTexture && !material.metallicTexture.empty());

	if (material.useRoughnessTexture && !material.roughnessTexture.empty())
	{
		instance->roughnessMetalImage = UploadTexture(device, material.roughnessTexture, false);
	}
	if (!instance->roughnessMetalImage)
	{
		uint8_t rough = static_cast<uint8_t>(material.roughness * 255);
		uint8_t metal = static_cast<uint8_t>(material.metallic * 255);
		instance->roughnessMetalImage = CreateFallback1x1(device, rough, metal, 0, 255);
	}
	else
	{
		instance->hasRoughnessMetalTexture = true;
	}
	instance->roughnessMetalTexture = std::make_unique<core::gpu::Texture>(device, *instance->roughnessMetalImage);

	BindAndUpdate(device, *instance, dsLayout);
	return instance;
}

std::unique_ptr<MaterialInstance> loaders::MaterialLoader::CreateDefault(
	Device& device,
	const core::gpu::DescriptorSetLayout* dsLayout)
{
	graphics::resources::object::Material mat;
	return Load(device, mat, dsLayout);
}