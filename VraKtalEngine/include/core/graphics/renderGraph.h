#ifndef VRAKTAL_CORE_GRAPHICS_RENDER_GRAPH_H
#define VRAKTAL_CORE_GRAPHICS_RENDER_GRAPH_H
#pragma once

#include <vector>
#include <functional>
#include <unordered_map>

#include <core/rhi/commandBuffer.h>
#include <core/rhi/image.h>

using namespace core::rhi;

namespace core::graphics
{
	enum class Layout
	{
		Undefined,
		ColorAttachment,
		DepthStencilAttachment,
		ShaderReadOnly,
		TransferSrc,
		TransferDst
	};

	enum class AccessFlags
	{
		None,
		Read,
		Write,
		ReadWrite,
		Transfer
	};

	// Used descriptor when creating new resource
	struct ResourceDescriptor
	{
		enum class Type { Image, Buffer } type;

		union
		{
			struct { uint32_t width; uint32_t height; } imageSize;
			size_t bufferSize;
		};
	};

	struct Resource
	{
		uint32_t id;
		Layout currentLayout;
		AccessFlags lastAccess;
		Image* image = nullptr;
		ResourceDescriptor descriptor; //--> if fresh resource
	};

	struct Pass
	{
		std::vector<Resource*> inputs;
		std::vector<Resource*> outputs;
		std::function<void(CommandBuffer&)> callback;
	};

	class RenderGraph
	{
	public:
		// Creating fresh resource and returns it.
		virtual Resource AddResource(const ResourceDescriptor& desc, Layout initialLayout);

		// Using existing resource by its id.
		virtual void AddResourceReference(uint32_t existingId, Layout initialLayout);

		// AddPass: add a rendering pass to the graph.
		virtual void AddPass(const Pass& pass);

		// Compile: compute execution order and prepare resource transitions.
		virtual void Compile();

		// Execute: run each pass in order, applying layout/access transitions and invoking callbacks.
		virtual DDvoid Execute(CommandBuffer& commandBuffer);

	private:
		std::vector<Pass> m_passes;
		std::vector<Pass*> m_executionOrder;
		std::unordered_map<uint32_t, Resource> m_resources;
		uint32_t m_nextResourcesId = 0;
	};
}

#endif //VRAKTAL_CORE_GRAPHICS_RENDER_GRAPH_H
