#ifndef VRAKTAL_CORE_GPU_RENDER_GRAPH_H
#define VRAKTAL_CORE_GPU_RENDER_GRAPH_H
#pragma once

#include <vector>
#include <functional>
#include <core/gpu/commandBuffer.h>
#include <core/gpu/image.h>

using namespace rhi::core::gpu;

namespace core::gpu
{
	// A pass defines resources it reads (inputs) and writes (outputs).
	// Each pass has a callback to record commands into a CommandBuffer.
	struct Pass
	{
		std::vector<Resource*> inputs;
		std::vector<Resource*> outputs;
		std::function<void(CommandBuffer&)> callback;
	};

	// Wraps a GPU resource (Image) with logical state:
	// layout, last access flags, and resource usage status.
	struct Resource
	{
		Image* image;
		Layout currentLayout = Layout::Undefined;
		AccessFlags lastAccess = AccessFlags::None;
		ResourceStatus status = ResourceStatus::Unused;
	};

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

	enum class ResourceStatus
	{
		Unused,
		InUse,
		Released,
		Freed
	};

	class RenderGraph
	{
	public:
		// AddPass: add a rendering pass to the graph.
		void AddPass(const Pass& pass);

		// Compile: compute execution order and prepare resource transitions.
		void Compile();

		// Execute: run each pass in order, applying layout/access transitions and invoking callbacks.
		void Execute(CommandBuffer& commandBuffer);

	private:
		std::vector<Pass> m_passes;
		std::vector<Pass*> m_executionOrder;
	};
}

#endif //VRAKTA_CORE_GPU_RENDER_GRAPH_H
