#ifndef VRAKTAL_CORE_GPU_RENDER_GRAPH_H
#define VRAKTAL_CORE_GPU_RENDER_GRAPH_H
#pragma once

#include <vector>
#include <functional>
#include <unordered_map>
#include <core/gpu/commandBuffer.h>
#include <core/gpu/image.h>

using namespace rhi::core::gpu;

namespace core::gpu
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

		uint32_t width = 0;   
		uint32_t height = 0;  
		uint32_t size = 0;    
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
		Resource AddResource(const ResourceDescriptor& desc, Layout initialLayout);

		// Using existing resource by its id.
		void AddResourceReference(uint32_t existingId, Layout initialLayout);

		// AddPass: add a rendering pass to the graph.
		void AddPass(const Pass& pass);

		// Compile: compute execution order and prepare resource transitions.
		void Compile();

		// Execute: run each pass in order, applying layout/access transitions and invoking callbacks.
		void Execute(CommandBuffer& commandBuffer);

	private:
		std::vector<Pass> m_passes;
		std::vector<Pass*> m_executionOrder;
		std::unordered_map<uint32_t, Resource> m_resources;
		uint32_t m_nextResourcesId = 0;
	};
}

#endif //VRAKTA_CORE_GPU_RENDER_GRAPH_H
