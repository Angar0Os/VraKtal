#include "renderGraph.h"
#include <stdexcept>
#include <iostream>

using namespace graphics;

static void CreateImageAndView(VkDevice device, VmaAllocator allocator, const RGTextureDesc& desc, VkImage& image, VkImageView& view, VmaAllocation& allocation)
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

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateImage(allocator, &imgInfo, &allocInfo, &image, &allocation, nullptr) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create RG Image");
    }

    VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = desc.format;
    viewInfo.subresourceRange.aspectMask = (desc.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &viewInfo, nullptr, &view) != VK_SUCCESS)
    {
        vmaDestroyImage(allocator, image, allocation);
        throw std::runtime_error("Failed to create RG image view");
    }
}

RenderGraph::RenderGraph(VkDevice device, VmaAllocator allocator, uint32_t width, uint32_t height)
    : m_device(device), m_allocator(allocator), m_width(width), m_height(height)
{
}

RenderGraph::~RenderGraph()
{
    Cleanup();
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
        CreateTextureIfNeeded(tex);
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

void RenderGraph::Clear()
{
    m_passes.clear();
}

void RenderGraph::Resize(uint32_t width, uint32_t height)
{
    if (m_width == width && m_height == height)
    {
        return; 
    }

    m_width = width;
    m_height = height;

    for (auto& [name, tex] : m_textures)
    {
        if (tex.image != VK_NULL_HANDLE)
        {
            DestroyTexture(tex);
        }
        tex.desc.width = width;
        tex.desc.height = height;
    }
}

void RenderGraph::Cleanup()
{
    for (auto& [_, tex] : m_textures)
    {
        DestroyTexture(tex);
    }
    m_textures.clear();
    m_passes.clear();
}

bool RenderGraph::IsTextureCompatible(const std::string& name, const RGTextureDesc& desc) const
{
    auto it = m_textures.find(name);
    if (it == m_textures.end())
    {
        return false;
    }

    const RGTextureDesc& existing = it->second.desc;
    return existing.width == desc.width &&
        existing.height == desc.height &&
        existing.format == desc.format &&
        existing.usage == desc.usage;
}

void RenderGraph::DestroyTexture(RGTexture& texture)
{
    if (texture.view != VK_NULL_HANDLE)
    {
        vkDestroyImageView(m_device, texture.view, nullptr);
        texture.view = VK_NULL_HANDLE;
    }
    if (texture.image != VK_NULL_HANDLE)
    {
        vmaDestroyImage(m_allocator, texture.image, texture.allocation);
        texture.image = VK_NULL_HANDLE;
        texture.allocation = VK_NULL_HANDLE;
    }
}

void RenderGraph::CreateTextureIfNeeded(RGTexture& texture)
{
    if (texture.image == VK_NULL_HANDLE)
    {
        texture.desc.width = m_width;
        texture.desc.height = m_height;

        CreateImageAndView(m_device, m_allocator, texture.desc, texture.image, texture.view, texture.allocation);
    }
}

RGTexture* RenderGraph::GetTexture(const std::string& name)
{
    auto it = m_textures.find(name);
    if (it != m_textures.end())
    {
        return &it->second;
    }
    return nullptr;
}