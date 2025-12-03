#ifndef VRAKTAL_CORE_GPU_VULKAN_COMMANDBUFFER_H
#define VRAKTAL_CORE_GPU_VULKAN_COMMANDBUFFER_H
#pragma once

#include <core/gpu/commandBuffer.h>

#include <vulkan/vulkan_raii.hpp>

namespace core::gpu
{
	struct CommandBuffer::Impl
	{
		CommandBuffer& parent;
		vk::raii::CommandBuffers commandBuffers;
		bool isSingleTime;
		uint32_t currentIndex;

		explicit Impl(CommandBuffer& p, const core::gpu::Device* device, const SCommandBufferCreateInfo& info);
		~Impl();

		vk::raii::CommandBuffer& GetCommandBuffer(uint32_t index = 0);
		const vk::raii::CommandBuffer& GetCommandBuffer(uint32_t index = 0) const;

		uint32_t GetCount() const;
		bool IsSingleTime() const;

		void Begin(uint32_t index);
		void End(uint32_t index);
		void Submit(const core::gpu::Device* device, void* waitSemaphore = nullptr, void* signalSemaphore = nullptr, void* fence = nullptr);
		void SubmitAndWait(const core::gpu::Device* device);

		void BindPipeline(const core::gpu::Pipeline* device);
		void BindVertexBuffer(const core::gpu::Buffer* buffer, size_t offset);
		void BindIndexBuffer(const core::gpu::Buffer* buffer, size_t offset);
		void BindDescriptorSets(const core::gpu::Device* device, const core::gpu::DescriptorSet* descriptorSet, uint32_t frameIndex, uint32_t firstSet);
		void SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth);
		void SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height);
		void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
		void BeginRendering(uint32_t width, uint32_t height, void* colorImageView, void* depthImageView);
		void EndRendering();

		void CopyBuffer(const core::gpu::Buffer* srcBuffer, const core::gpu::Buffer* dstBuffer, size_t size);

		void PushConstants(const core::gpu::Pipeline* pipeline, uint32_t stageFlags, uint32_t offset, uint32_t size, const void* pValues);

		void TransitionImageLayout(const core::gpu::Image* image,
			vk::ImageLayout oldLayout,
			vk::ImageLayout newLayout,
			vk::AccessFlags srcAccess,
			vk::AccessFlags dstAccess,
			vk::PipelineStageFlags srcStage,
			vk::PipelineStageFlags dstStage,
			bool isDepth);

		void ResolveImage(const core::gpu::Image* srcImage, const core::gpu::Image* dstImage, uint32_t width, uint32_t height);

		void BuildAccelerationStructure(const core::gpu::AccelerationStructure* accelerationStructure);

		void AccelerationStructureBarrier();

		void TraceRays(const core::gpu::Device* device,
			void* raygenSBT, uint32_t raygenOffset, uint32_t raygenStride,
			void* missSBT, uint32_t missOffset, uint32_t missStride, uint32_t missCount,
			void* hitSBT, uint32_t hitOffset, uint32_t hitStride, uint32_t hitCount,
			void* callableSBT, uint32_t callableOffset, uint32_t callableStride, uint32_t callableCount,
			uint32_t width, uint32_t height, uint32_t depth);

		void BindRayTracingPipeline(const core::gpu::Pipeline* pipeline);
	};
}

#endif //VRAKTAL_CORE_GPU_VULKAN_COMMANDBUFFER_H