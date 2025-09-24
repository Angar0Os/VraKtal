#ifndef VRAKTAL_GRAPHICS_RENDER_GRAPH_H
#define VRAKTAL_GRAPHICS_RENDER_GRAPH_H
#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <unordered_map>
#include <functional>

namespace graphics
{
	struct RGTextureDesc
	{
		uint32_t width, height;
		VkFormat format;
		VkImageUsageFlags usage;
	};

	struct RGTexture
	{
		std::string name;
		RGTextureDesc desc;
		VkImage image = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
	};

	using RGTextureHandle = size_t;

	struct RenderGraphResources
	{
		std::unordered_map<std::string, RGTexture*> textures;
		
		VkImageView getView(const std::string& name) const
		{
			auto it = textures.find(name);
			return (it != textures.end()) ? it->second->view : VK_NULL_HANDLE;
		}
	};

	using PassCallback = std::function<void(VkCommandBuffer, const RenderGraphResources&)>;

	struct RenderPassNode
	{
		std::string name;
		std::vector<std::string> reads;
		std::vector<std::string> writes;
		PassCallback execute;
	};

	class RenderGraph
	{
	public:
		RenderGraph(VkDevice device, uint32_t width, uint32_t height);
		~RenderGraph();

		void AddTexture(const std::string& name, const RGTextureDesc& desc);
		void AddPass(const RenderPassNode&node);

		void Compile();
		void Execute(VkCommandBuffer cmd);

	private:
		VkDevice m_device;
		uint32_t m_width, m_height;

		std::unordered_map<std::string, RGTexture> m_textures;
		std::vector<RenderPassNode> m_passes;
	};
}

#endif //VRAKTAL_GRAPHICS_RENDER_GRAPH_H