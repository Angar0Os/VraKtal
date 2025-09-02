#include "textureLoader.h"
#include "gltfLoader.h"
#include "../../core/gpu/gpuDevice_impl_glfw_vulkan.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stdexcept>
#include <iostream>
#include <cstring>

using namespace graphics::loaders;
using namespace graphics::resources;
using namespace rhi::vulkan;

VkFormat ChooseFormat(bool srgb)
{
	return srgb ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
}

TextureLoader::TextureLoader(GpuDeviceVulkan& device)
	: m_device(device)
{
}

TextureLoader::~TextureLoader()
{
}

TextureGpu TextureLoader::CreateWhiteFallback()
{
	TextureGpu tex{};
	tex.width = 1;
	tex.height = 1;
	unsigned char pixel[4] = { 255, 255, 255, 255 };
	VkFormat format = ChooseFormat(true);

	VkImageCreateInfo imgInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imgInfo.imageType = VK_IMAGE_TYPE_2D;
	imgInfo.format = format;
	imgInfo.extent = { tex.width, tex.height };
	imgInfo.mipLevels = 1;
	imgInfo.arrayLayers = 1;
	imgInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imgInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imgInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	if (vmaCreateImage(m_device.Allocator(), &imgInfo, &allocInfo, &tex.image, &tex.allocation, nullptr) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create fallback image");
	}

	VkBuffer staging;
	VmaAllocation stagingAlloc;
	VkBufferCreateInfo bufInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufInfo.size = sizeof(pixel);
	bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VmaAllocationCreateInfo stagingAllocInfo{};
	stagingAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
	vmaCreateBuffer(m_device.Allocator(), &bufInfo, &stagingAllocInfo, &staging, &stagingAlloc, nullptr);

	void* mapped;
	vmaMapMemory(m_device.Allocator(), stagingAlloc, &mapped);
	std::memcpy(mapped, pixel, sizeof(pixel));
	vmaUnmapMemory(m_device.Allocator(), stagingAlloc);

	VkCommandBufferAllocateInfo allocInfoCmd{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocInfoCmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfoCmd.commandBufferCount = 1;
	allocInfoCmd.commandPool = m_device.CommandPool();
	VkCommandBuffer cmd;
	vkAllocateCommandBuffers(m_device.Device(), &allocInfoCmd, &cmd);

	VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(cmd, &beginInfo);

	VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = tex.image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
		0, nullptr, 0, nullptr, 1, &barrier);

	VkBufferImageCopy copy{};
	copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copy.imageSubresource.mipLevel = 0;
	copy.imageSubresource.baseArrayLayer = 0;
	copy.imageSubresource.layerCount = 1;
	copy.imageExtent = { tex.width, tex.height, 1 };
	vkCmdCopyBufferToImage(cmd, staging, tex.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

	VkImageMemoryBarrier barrier2 = barrier;
	barrier2.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier2.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier2.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier2.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr, 0, nullptr, 1, &barrier2);

	vkEndCommandBuffer(cmd);
	VkSubmitInfo submit{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &cmd;
	vkQueueSubmit(m_device.GraphicsQueue(), 1, &submit, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_device.GraphicsQueue());
	vkFreeCommandBuffers(m_device.Device(), m_device.CommandPool(), 1, &cmd);

	vmaDestroyBuffer(m_device.Allocator(), staging, stagingAlloc);

	VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.image = tex.image;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.layerCount = 1;
	if (vkCreateImageView(m_device.Device(), &viewInfo, nullptr, &tex.view) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create fallback image view");
	}

	VkSamplerCreateInfo samp{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
	samp.magFilter = VK_FILTER_LINEAR;
	samp.minFilter = VK_FILTER_LINEAR;
	samp.addressModeU = samp.addressModeV = samp.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samp.maxLod = 0.0f;
	if (vkCreateSampler(m_device.Device(), &samp, nullptr, &tex.sampler) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create fallback sampler");
	}

	return tex;
}

TextureGpu TextureLoader::LoadTextureFile(const std::string& path, bool srgb)
{
	if (path == "<embedded>")
	{
		std::cerr << "Embedded image handling not implemented" << std::endl;
		return CreateWhiteFallback();
	}

	int w, h, comp;
	stbi_uc* data = stbi_load(path.c_str(), &w, &h, &comp, STBI_rgb_alpha);
	if (!data)
	{
		std::cerr << "Error during texture loading" << path << std::endl;
		return CreateWhiteFallback();
	}
	
	size_t dataSize = size_t(w) * size_t(h) * 4;

	TextureGpu tex{};
	tex.width = (uint32_t)w;
	tex.height = (uint32_t)h;

	VkFormat format = ChooseFormat(srgb);


	VkImageCreateInfo imgInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imgInfo.imageType = VK_IMAGE_TYPE_2D;
	imgInfo.format = format;
	imgInfo.extent = { tex.width, tex.height, 1 };
	imgInfo.mipLevels = 1;
	imgInfo.arrayLayers = 1;
	imgInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imgInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imgInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	if (vmaCreateImage(m_device.Allocator(), &imgInfo, &allocInfo, &tex.image, &tex.allocation, nullptr) != VK_SUCCESS) 
	{
		stbi_image_free(data);
		throw std::runtime_error("Failed to create image");
	}

	VkBuffer staging;
	VmaAllocation stagingAlloc;
	VkBufferCreateInfo bufInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufInfo.size = dataSize;
	bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VmaAllocationCreateInfo stagingAllocInfo{};
	stagingAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
	vmaCreateBuffer(m_device.Allocator(), &bufInfo, &stagingAllocInfo, &staging, &stagingAlloc, nullptr);

	void* mapped;
	vmaMapMemory(m_device.Allocator(), stagingAlloc, &mapped);
	std::memcpy(mapped, data, dataSize);
	vmaUnmapMemory(m_device.Allocator(), stagingAlloc);
	stbi_image_free(data);

	VkCommandBufferAllocateInfo allocInfoCmd{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocInfoCmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfoCmd.commandBufferCount = 1;
	allocInfoCmd.commandPool = m_device.CommandPool();
	VkCommandBuffer cmd;
	vkAllocateCommandBuffers(m_device.Device(), &allocInfoCmd, &cmd);

	VkCommandBufferBeginInfo begin{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(cmd, &begin);

	VkImageMemoryBarrier toTransfer{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	toTransfer.image = tex.image;
	toTransfer.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	toTransfer.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	toTransfer.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	toTransfer.subresourceRange.levelCount = 1;
	toTransfer.subresourceRange.layerCount = 1;
	toTransfer.srcAccessMask = 0;
	toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		0, 0, nullptr, 0, nullptr, 1, &toTransfer);

	VkBufferImageCopy copy{};
	copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copy.imageSubresource.mipLevel = 0;
	copy.imageSubresource.layerCount = 1;
	copy.imageExtent = { tex.width, tex.height, 1 };
	vkCmdCopyBufferToImage(cmd, staging, tex.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

	VkImageMemoryBarrier toShader = toTransfer;
	toShader.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	toShader.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	toShader.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	toShader.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0, 0, nullptr, 0, nullptr, 1, &toShader);

	vkEndCommandBuffer(cmd);
	VkSubmitInfo submit{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &cmd;
	vkQueueSubmit(m_device.GraphicsQueue(), 1, &submit, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_device.GraphicsQueue());
	vkFreeCommandBuffers(m_device.Device(), m_device.CommandPool(), 1, &cmd);
	vmaDestroyBuffer(m_device.Allocator(), staging, stagingAlloc);

	VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.image = tex.image;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.layerCount = 1;
	if (vkCreateImageView(m_device.Device(), &viewInfo, nullptr, &tex.view) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create image view");
	}

	VkSamplerCreateInfo samp{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
	samp.magFilter = VK_FILTER_LINEAR;
	samp.minFilter = VK_FILTER_LINEAR;
	samp.addressModeU = samp.addressModeV = samp.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samp.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samp.maxLod = 0.0f;
	if (vkCreateSampler(m_device.Device(), &samp, nullptr, &tex.sampler) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create sampler");
	}

	return tex;
}

void TextureLoader::LoadSceneTextures(const graphics::scene::Scene& scene, std::vector<TextureGpu>& texturesOut) 
{
	texturesOut.clear();
	texturesOut.reserve(scene.textures.size());
	for (auto& path : scene.textures) 
	{
		texturesOut.push_back(LoadTextureFile(path, true));
	}
	if (texturesOut.empty()) 
	{
		texturesOut.push_back(CreateWhiteFallback());
	}
}