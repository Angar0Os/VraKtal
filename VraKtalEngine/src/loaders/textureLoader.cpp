#include <loaders/textureLoader.h>

#include <core/gpu/buffer.h>
#include <core/gpu/commandBuffer.h>
#include <core/enum.h>

#include <stb_image.h>

#include <string>

#include <iostream>
#include <memory>
#include <core/gpu/image.h>
#include <loaders/loaderBase.h>

using namespace core;
using namespace core::gpu;
using namespace loaders;

TextureLoader::TextureLoader(core::gpu::Device* _device) : m_device(*_device) {}

loaders::TextureLoader::~TextureLoader() {}

std::shared_ptr<void> TextureLoader::Load(const std::string& path, const LoadOptions* options)
{
	bool isSRGB = false;
	if (auto textureOptions = dynamic_cast<const TextureLoadOptions*>(options))
	{
		isSRGB = textureOptions->isSRGB;
	}

	int width, height, channels;
	stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	if (!pixels)
	{
		std::cerr << "MaterialLoader: failed to load texture " << path << ", using fallback\n";
		return nullptr;
	}
	size_t imageSize = static_cast<size_t>(width) * height * 4;
	SBufferCreateInfo stagingInfo{
		.size = imageSize,
		.usage = EBufferUsage::TransferSrc,
		.memoryProperties = EMemoryProperty::HostVisible | EMemoryProperty::HostCoherent
	};
	auto staging = std::make_unique<Buffer>(&m_device, stagingInfo);
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
	auto image = std::make_unique<Image>(&m_device, imageInfo);

	SCommandBufferCreateInfo cmdInfo{
		.device = &m_device,
		.level = ECommandBufferLevel::Primary,
		.count = 1,
		.singleTime = true
	};

	CommandBuffer cmd(&m_device, cmdInfo);
	cmd.Begin(0);
	cmd.TransitionImageLayout(image.get(), ImageLayout::Undefined, ImageLayout::TransferDst, false);
	cmd.CopyBufferToImage(staging.get(), image.get(), width, height);
	cmd.TransitionImageLayout(image.get(), ImageLayout::TransferDst, ImageLayout::ShaderReadOnly, false);
	cmd.End(0);
	cmd.SubmitImmediate(&m_device);

	return image;
}
