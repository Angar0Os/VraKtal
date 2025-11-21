#include "../src/core/gpu/vulkan/texture_impl.h"
#include <core/gpu/image.h>
#include <core/gpu/buffer.h>
#include <core/gpu/commandBuffer.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stdexcept>
#include <cmath>
#include <fstream>
#include <algorithm>
#include <iostream>

core::gpu::Texture::Impl::Impl(core::gpu::Texture& p, vk::raii::Device& dev,
	vk::raii::PhysicalDevice& physDev, vk::raii::Queue& q, vk::raii::CommandPool& pool,
	const TextureCreateInfo& info)
	: parent(p), device(dev), physicalDevice(physDev), queue(q), commandPool(pool),
	width(0), height(0), mipLevels(1)
{
	try {
		LoadFromFile(info);
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load texture from file: " << e.what() << std::endl;
		width = 1;
		height = 1;
		image = CreateSolidColorImage(device, physicalDevice, queue, commandPool,
			1.0f, 0.0f, 1.0f, 1.0f, TextureFormat::RGBA8_SRGB);
	}
}

core::gpu::Texture::Impl::~Impl() = default;

void core::gpu::Texture::Impl::LoadFromFile(const TextureCreateInfo& info)
{
	stbi_set_flip_vertically_on_load(info.flipVertically);

	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(info.filepath.c_str(), &texWidth, &texHeight,
		&texChannels, STBI_rgb_alpha);

	if (!pixels)
	{
		throw std::runtime_error("Failed to load texture image: " + info.filepath);
	}

	width = static_cast<uint32_t>(texWidth);
	height = static_cast<uint32_t>(texHeight);

	if (info.generateMipmaps)
	{
		mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
	}
	else
	{
		mipLevels = 1;
	}

	vk::DeviceSize imageSize = texWidth * texHeight * 4;

	BufferCreateInfo stagingBufferInfo{};
	stagingBufferInfo.size = imageSize;
	stagingBufferInfo.usage = BufferUsage::TransferSrc;
	stagingBufferInfo.memoryProperties = MemoryProperty::HostVisible | MemoryProperty::HostCoherent;

	Buffer stagingBuffer(&device, &physicalDevice, stagingBufferInfo);
	stagingBuffer.CopyFrom(pixels, imageSize, 0);

	stbi_image_free(pixels);

	ImageCreateInfo imageInfo{};
	imageInfo.width = width;
	imageInfo.height = height;
	imageInfo.mipLevels = mipLevels;
	imageInfo.format = info.format;
	imageInfo.tiling = ImageTiling::Optimal;
	imageInfo.usage = ImageUsage::TransferSrc | ImageUsage::TransferDst | ImageUsage::Sampled;
	imageInfo.memoryProperties = MemoryProperty::DeviceLocal;
	imageInfo.samples = SampleCount::e1;

	image = std::make_unique<Image>(&device, &physicalDevice, imageInfo);

	auto commandBuffer = BeginSingleTimeCommands();

	image->TransitionLayout(*commandBuffer, ImageLayout::TransferDst, ImageLayout::TransferDst, 1);
	image->CopyFromBuffer(*commandBuffer, stagingBuffer, width, height);

	if (info.generateMipmaps)
	{
		image->GenerateMipmaps(*commandBuffer, width, height, mipLevels);
	}
	else
	{
		image->TransitionLayout(*commandBuffer, ImageLayout::TransferDst,
			ImageLayout::ShaderReadOnly, 1);
	}

	EndSingleTimeCommands(commandBuffer);

	ImageViewCreateInfo viewInfo{};
	viewInfo.format = info.format;
	viewInfo.baseMipLevel = 0;
	viewInfo.levelCount = mipLevels;
	viewInfo.baseArrayLayer = 0;
	viewInfo.layerCount = 1;
	viewInfo.isDepth = false;

	image->CreateView(viewInfo);
}

std::unique_ptr<core::gpu::CommandBuffer> core::gpu::Texture::Impl::BeginSingleTimeCommands()
{
	CommandBufferCreateInfo cmdInfo{};
	cmdInfo.commandPool = &commandPool;
	cmdInfo.level = CommandBufferLevel::Primary;
	cmdInfo.count = 1;
	cmdInfo.singleTime = true;

	auto commandBuffer = std::make_unique<CommandBuffer>(&device, &queue, cmdInfo);
	commandBuffer->Begin(0);

	return commandBuffer;
}

void core::gpu::Texture::Impl::EndSingleTimeCommands(std::unique_ptr<CommandBuffer>& commandBuffer)
{
	commandBuffer->End(0);
	commandBuffer->SubmitAndWait();
}

core::gpu::Image* core::gpu::Texture::Impl::GetImage() const
{
	return image.get();
}

core::gpu::Texture::Texture(void* device, void* physicalDevice, void* queue,
	void* commandPool, const TextureCreateInfo& info)
{
	auto& vkDevice = *static_cast<vk::raii::Device*>(device);
	auto& vkPhysicalDevice = *static_cast<vk::raii::PhysicalDevice*>(physicalDevice);
	auto& vkQueue = *static_cast<vk::raii::Queue*>(queue);
	auto& vkCommandPool = *static_cast<vk::raii::CommandPool*>(commandPool);

	m_impl = std::make_unique<Impl>(*this, vkDevice, vkPhysicalDevice, vkQueue,
		vkCommandPool, info);
}

core::gpu::Texture::Impl::Impl(core::gpu::Texture& p, vk::raii::Device& dev,
	vk::raii::PhysicalDevice& physDev, vk::raii::Queue& q, vk::raii::CommandPool& pool,
	float r, float g, float b, float a, TextureFormat format)
	: parent(p), device(dev), physicalDevice(physDev), queue(q), commandPool(pool),
	width(1), height(1), mipLevels(1)
{
	image = CreateSolidColorImage(device, physicalDevice, queue, commandPool, r, g, b, a, format);
}

std::unique_ptr<core::gpu::Image> core::gpu::Texture::Impl::CreateSolidColorImage(
	vk::raii::Device& device,
	vk::raii::PhysicalDevice& physicalDevice,
	vk::raii::Queue& queue,
	vk::raii::CommandPool& commandPool,
	float r, float g, float b, float a,
	TextureFormat format)
{
	const uint32_t width = 1;
	const uint32_t height = 1;

	uint8_t pixelData[4] = {
		static_cast<uint8_t>(std::clamp(r, 0.0f, 1.0f) * 255.0f),
		static_cast<uint8_t>(std::clamp(g, 0.0f, 1.0f) * 255.0f),
		static_cast<uint8_t>(std::clamp(b, 0.0f, 1.0f) * 255.0f),
		static_cast<uint8_t>(std::clamp(a, 0.0f, 1.0f) * 255.0f)
	};

	vk::DeviceSize imageSize = sizeof(pixelData);

	BufferCreateInfo stagingBufferInfo{
		.size = imageSize,
		.usage = BufferUsage::TransferSrc,
		.memoryProperties = MemoryProperty::HostVisible | MemoryProperty::HostCoherent
	};

	Buffer stagingBuffer(&device, &physicalDevice, stagingBufferInfo);
	stagingBuffer.CopyFrom(pixelData, imageSize, 0);

	ImageCreateInfo imageInfo{
		.width = width,
		.height = height,
		.mipLevels = 1,
		.arrayLayers = 1,
		.format = format,
		.tiling = ImageTiling::Optimal,
		.usage = ImageUsage::TransferDst | ImageUsage::Sampled,
		.memoryProperties = MemoryProperty::DeviceLocal,
		.samples = SampleCount::e1
	};

	auto image = std::make_unique<Image>(&device, &physicalDevice, imageInfo);

	CommandBufferCreateInfo cmdInfo{
		.commandPool = &commandPool,
		.level = CommandBufferLevel::Primary,
		.count = 1,
		.singleTime = true
	};
	CommandBuffer cmdBuffer(&device, &queue, cmdInfo);

	cmdBuffer.Begin(0);
	image->TransitionLayout(cmdBuffer, ImageLayout::Undefined, ImageLayout::TransferDst, 1);
	image->CopyFromBuffer(cmdBuffer, stagingBuffer, width, height);
	image->TransitionLayout(cmdBuffer, ImageLayout::TransferDst, ImageLayout::ShaderReadOnly, 1);
	cmdBuffer.End(0);
	cmdBuffer.SubmitAndWait();

	ImageViewCreateInfo viewInfo{
		.format = format,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1,
		.isDepth = false
	};
	image->CreateView(viewInfo);

	return image;
}

bool core::gpu::Texture::Impl::LoadTextureIfExists(const std::string& filepath)
{
	std::ifstream file(filepath);
	if (!file.good())
	{
		std::cerr << "Texture file not found: " << filepath << ", using default color" << std::endl;
		return false;
	}

	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight,
		&texChannels, STBI_rgb_alpha);

	if (!pixels)
	{
		std::cerr << "Failed to load texture image: " << filepath << std::endl;
		return false;
	}

	width = static_cast<uint32_t>(texWidth);
	height = static_cast<uint32_t>(texHeight);

	mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	vk::DeviceSize imageSize = texWidth * texHeight * 4;

	BufferCreateInfo stagingBufferInfo{
		.size = imageSize,
		.usage = BufferUsage::TransferSrc,
		.memoryProperties = MemoryProperty::HostVisible | MemoryProperty::HostCoherent
	};

	Buffer stagingBuffer(&device, &physicalDevice, stagingBufferInfo);

	stagingBuffer.CopyFrom(pixels, imageSize, 0);

	stbi_image_free(pixels);

	ImageCreateInfo imageInfo{
		.width = width,
		.height = height,
		.mipLevels = mipLevels,
		.arrayLayers = 1,
		.format = TextureFormat::RGBA8_SRGB,
		.tiling = ImageTiling::Optimal,
		.usage = ImageUsage::TransferSrc | ImageUsage::TransferDst | ImageUsage::Sampled,
		.memoryProperties = MemoryProperty::DeviceLocal,
		.samples = SampleCount::e1
	};

	image = std::make_unique<Image>(&device, &physicalDevice, imageInfo);

	CommandBufferCreateInfo cmdInfo{
		.commandPool = &commandPool,
		.level = CommandBufferLevel::Primary,
		.count = 1,
		.singleTime = true
	};
	CommandBuffer cmdBuffer(&device, &queue, cmdInfo);

	cmdBuffer.Begin(0);

	image->TransitionLayout(cmdBuffer, ImageLayout::Undefined,
		ImageLayout::TransferDst, mipLevels);

	image->CopyFromBuffer(cmdBuffer, stagingBuffer, width, height);

	image->GenerateMipmaps(cmdBuffer, width, height, mipLevels);

	cmdBuffer.End(0);
	cmdBuffer.SubmitAndWait();

	ImageViewCreateInfo viewInfo{
		.format = TextureFormat::RGBA8_SRGB,
		.baseMipLevel = 0,
		.levelCount = mipLevels,
		.baseArrayLayer = 0,
		.layerCount = 1,
		.isDepth = false
	};
	image->CreateView(viewInfo);

	std::cout << "Successfully loaded texture: " << filepath
		<< " (" << width << "x" << height << ", " << mipLevels << " mips)" << std::endl;
	return true;
}

core::gpu::Texture::~Texture() = default;

core::gpu::Texture::Texture(Texture&&) noexcept = default;
core::gpu::Texture& core::gpu::Texture::operator=(Texture&&) noexcept = default;

core::gpu::Image* core::gpu::Texture::GetImage() const
{
	return m_impl->GetImage();
}

void* core::gpu::Texture::GetImageView() const
{
	return m_impl->GetImage()->GetViewHandle();
}

uint32_t core::gpu::Texture::GetWidth() const
{
	return m_impl->GetWidth();
}

uint32_t core::gpu::Texture::GetHeight() const
{
	return m_impl->GetHeight();
}

uint32_t core::gpu::Texture::GetMipLevels() const
{
	return m_impl->GetMipLevels();
}

core::gpu::Texture::Impl& core::gpu::Texture::GetImpl()
{
	return *m_impl;
}

const core::gpu::Texture::Impl& core::gpu::Texture::GetImpl() const
{
	return *m_impl;
}

core::gpu::Texture::Texture(void* device, void* physicalDevice, void* queue,
	void* commandPool, float r, float g, float b, float a, TextureFormat format)
{
	auto& vkDevice = *static_cast<vk::raii::Device*>(device);
	auto& vkPhysicalDevice = *static_cast<vk::raii::PhysicalDevice*>(physicalDevice);
	auto& vkQueue = *static_cast<vk::raii::Queue*>(queue);
	auto& vkCommandPool = *static_cast<vk::raii::CommandPool*>(commandPool);

	m_impl = std::make_unique<Impl>(*this, vkDevice, vkPhysicalDevice, vkQueue,
		vkCommandPool, r, g, b, a, format);
}

bool core::gpu::Texture::LoadTextureIfExists(const std::string& filepath)
{
	return m_impl->LoadTextureIfExists(filepath);
}