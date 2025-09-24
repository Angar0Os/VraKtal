#include "renderGraph.h"
#include <stdexcept>
#include <iostream>

using namespace graphics;

static void CreateImageAndView(VkDevice device, const RGTextureDesc& desc, VkImage& image, VkImageView& view)
{
	VkImageCreateInfo imgInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imgInfo.imageType = VK_IMAGE_TYPE_2D;
	imgInfo.extent = { desc.width, desc.height, 1 };
	imgInfo.mipLevels = 1;
	imgInfo.arrayLayers = 1;
	imgInfo.format = desc.format;
	imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imgInfo.usage = desc.usage;
	imgInfo.samples = VK_SAMPLE_COUNT_1_BIT;

	if (vkCreateImage(device, &imgInfo, nullptr, &image) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create RG Image");
	}

	VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = desc.format;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(device, &viewInfo, nullptr, &view) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create RG image view");
	}
}

RenderGraph::RenderGraph(VkDevice device, uint32_t width, uint32_t height)
	: m_device(device), m_width(width), m_height(height)
{
}

RenderGraph::~RenderGraph()
{
	for (auto& [_, tex] : m_textures)
	{
		if (tex.view) vkDestroyImageView(m_device, tex.view, nullptr);
		if (tex.image) vkDestroyImage(m_device, tex.image, nullptr);
	}
}

void RenderGraph::AddTexture(const std::string& name, const RGTextureDesc& desc)
{
	RGTexture tex;
	tex.name = name;
	tex.desc = desc;
	m_textures[name] = tex;
}

void RenderGraph::AddPass(const RenderPassNode& node)
{
	m_passes.push_back(node);
}

void RenderGraph::Compile()
{
	for (auto& [name, tex] : m_textures)
	{
		if (tex.image == VK_NULL_HANDLE)
		{
			CreateImageAndView(m_device, tex.desc, tex.image, tex.view);
		}
	}
}

void RenderGraph::Execute(VkCommandBuffer cmd)
{
	RenderGraphResources res;
	for (auto& [name, tex] : m_textures)
	{
		res.textures[name] = &tex;
	}

	for (auto& pass : m_passes)
	{
		pass.execute(cmd, res);
	}
}