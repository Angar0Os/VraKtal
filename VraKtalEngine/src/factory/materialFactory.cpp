#include <factory/materialFactory.h>
#include <core/manager/ressourceManager.h>

#include <core/gpu/buffer.h>
#include <core/gpu/commandBuffer.h>
#include <core/enum.h>

#include <stb_image.h>

#include <stdexcept>
#include <iostream>

#include <graphics/assets/material.h>
#include <graphics/renderer.h>
#include <graphics/pass.h>
#include <graphics/renderPass/gBufferPass.h>


using namespace core;
using namespace core::gpu;
using namespace graphics::resources;

factory::MaterialFactory::MaterialFactory(core::gpu::Device& device, graphics::Renderer& _renderer) : m_device(device) , m_renderer(_renderer) {}

std::unique_ptr<Image> factory::MaterialFactory::UploadTexture( // Moved to texture loader
	Device& device,
	const std::string& filepath,
	bool               isSRGB)
{
	int width, height, channels;
	stbi_uc* pixels = stbi_load(filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

	if (!pixels)
	{
		std::cerr << "MaterialFactory: failed to load texture " << filepath << ", using fallback\n";
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
	stbi_image_free(pixels); // relase data

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

std::unique_ptr<Image> factory::MaterialFactory::UploadHDRTexture(
	Device& device,
	const std::string& filepath)
{
	int width, height, channels;
	float* pixels = stbi_loadf(filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

	if (!pixels)
	{
		std::cerr << "MaterialFactory: failed to load HDR " << filepath << "\n";
		return nullptr;
	}

	size_t imageSize = static_cast<size_t>(width) * height * 4 * sizeof(float);

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
		.format = TextureFormat::RGBA32_Float,
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

std::unique_ptr<Image> factory::MaterialFactory::CreateFallback1x1(
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

void factory::MaterialFactory::BindAndUpdate(
	Device& device,
	Material& instance,
	const core::gpu::DescriptorSetLayout* dsLayout)
{
	instance.descriptorSet = std::make_unique<DescriptorSet>(&device, const_cast<DescriptorSetLayout*>(dsLayout));
	instance.descriptorSet->Bind(0, *instance.materialBuffer);
	instance.descriptorSet->Bind(1, *instance.albedoTexture);
	instance.descriptorSet->Bind(2, *instance.normalTexture);
	instance.descriptorSet->Bind(3, *instance.roughnessMetalTexture);
	instance.descriptorSet->Update(device);
}

graphics::resources::Material factory::MaterialFactory::Create(const graphics::assets::Material& asset)
{
	graphics::resources::Material material;
	material.name = asset.name;
	material.albedoColor = asset.albedo;
	material.metalness = asset.metallic;
	material.roughnessValue = asset.roughness;

	if (asset.albedoTexture.size() > 0 && !asset.albedoTexture.empty())
	{
		material.albedoImage = UploadTexture(m_device, asset.albedoTexture, true);
	}
	if (!material.albedoImage)
	{
		uint8_t r = static_cast<uint8_t>(asset.albedo.r * 255);
		uint8_t g = static_cast<uint8_t>(asset.albedo.g * 255);
		uint8_t b = static_cast<uint8_t>(asset.albedo.b * 255);
		material.albedoImage = CreateFallback1x1(m_device, r, g, b, 255);
	}
	else
	{
		material.hasAlbedoTexture = true;
	}
	material.albedoTexture = std::make_unique<core::gpu::Texture>(m_device, *material.albedoImage); // On creer les textures a partir des images ?

	if (asset.normalTexture.size() > 0 && !asset.normalTexture.empty())
	{
		material.normalImage = UploadTexture(m_device, asset.normalTexture, false);
	}
	if (!material.normalImage)
	{
		material.normalImage = CreateFallback1x1(m_device, 128, 128, 255, 255);
	}
	else
	{
		material.hasNormalTexture = true;
	}
	material.normalTexture = std::make_unique<core::gpu::Texture>(m_device, *material.normalImage);

	if (asset.roughnessTexture.size() > 0 && !asset.roughnessTexture.empty())
	{
		material.roughnessMetalImage = UploadTexture(m_device, asset.roughnessTexture, false);
	}
	if (!material.roughnessMetalImage)
	{
		uint8_t rough = static_cast<uint8_t>(asset.roughness * 255);
		uint8_t metal = static_cast<uint8_t>(asset.metallic * 255);
		material.roughnessMetalImage = CreateFallback1x1(m_device, rough, metal, 0, 255);
	}
	else
	{
		material.hasRoughnessMetalTexture = true;
	}
	material.roughnessMetalTexture = std::make_unique<core::gpu::Texture>(m_device, *material.roughnessMetalImage);
	material.gpuData.baseColor = glm::vec4(asset.albedo, 1.0f);
	material.gpuData.params = glm::vec4(
		asset.metallic,   // x
		asset.roughness,  // y
		material.hasAlbedoTexture ? 1.0f : 0.0f, // z
		material.hasNormalTexture ? 1.0f : 0.0f  // w
	);

	SBufferCreateInfo bufferInfo{
		.size = sizeof(MaterialGPUData),
		.usage = EBufferUsage::UniformBuffer,
		.memoryProperties = EMemoryProperty::HostVisible | EMemoryProperty::HostCoherent
	};

	material.materialBuffer =
		std::make_unique<core::gpu::Buffer>(&m_device, bufferInfo);

	material.materialBuffer->CopyFrom(
		&material.gpuData,
		sizeof(MaterialGPUData)
	);

	BindAndUpdate(m_device, material, m_renderer.GetPass<graphics::GBufferPass>("GBuffer")->GetMaterialLayout());
	return material;
}

std::shared_ptr<Material> factory::MaterialFactory::CreateMaterialInstance(
	Device& device,
	const graphics::assets::Material& material,
	const core::gpu::DescriptorSetLayout* dsLayout //depend -> domain (postEffect , surface)
)
{
	auto instance = std::make_shared<Material>();
	instance->name = material.name;
	instance->albedoColor = material.albedo;
	instance->metalness = material.metallic;
	instance->roughnessValue = material.roughness;

	if (material.albedoTexture.size() > 0 && !material.albedoTexture.empty())
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
	instance->albedoTexture = std::make_unique<core::gpu::Texture>(device, *instance->albedoImage); // On creer les textures a partir des images ?

	if (material.normalTexture.size() > 0 && !material.normalTexture.empty())
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

	if (material.roughnessTexture.size() > 0 && !material.roughnessTexture.empty())
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
	instance->gpuData.baseColor = glm::vec4(material.albedo, 1.0f);
	instance->gpuData.params = glm::vec4(
		material.metallic,   // x
		material.roughness,  // y
		instance->hasAlbedoTexture ? 1.0f : 0.0f, // z
		instance->hasNormalTexture ? 1.0f : 0.0f  // w
	);

	SBufferCreateInfo bufferInfo{
		.size = sizeof(MaterialGPUData),
		.usage = EBufferUsage::UniformBuffer,
		.memoryProperties = EMemoryProperty::HostVisible | EMemoryProperty::HostCoherent
	};

	instance->materialBuffer =
		std::make_unique<core::gpu::Buffer>(&device, bufferInfo);

	instance->materialBuffer->CopyFrom(
		&instance->gpuData,
		sizeof(MaterialGPUData)
	);

	BindAndUpdate(device, *instance, dsLayout);
	return instance;
}

std::shared_ptr<Material> factory::MaterialFactory::CreateDefault(
	Device& device,
	const core::gpu::DescriptorSetLayout* dsLayout)
{
	graphics::assets::Material mat;
	return nullptr;
}