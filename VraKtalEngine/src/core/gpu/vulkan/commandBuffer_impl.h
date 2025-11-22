#ifndef VRAKTAL_CORE_GPU_VULKAN_COMMANDBUFFER_H
#define VRAKTAL_CORE_GPU_VULKAN_COMMANDBUFFER_H
#pragma once

#include <core/gpu/commandBuffer.h>

#include <vulkan/vulkan_raii.hpp>
#include <vector>

namespace core::gpu
{
	struct CommandBuffer::Impl
	{
	private:
		CommandBuffer& parent;
		vk::raii::Device& device;
		vk::raii::Queue& queue;
		vk::raii::CommandPool& commandPool;

		std::vector<vk::raii::CommandBuffer> commandBuffers;
		bool isSingleTime;
		uint32_t currentIndex;

	public:
		explicit Impl(CommandBuffer& p, vk::raii::Device& dev, vk::raii::Queue& q,
			vk::raii::CommandPool& pool, const CommandBufferCreateInfo& info);
		~Impl();

		vk::raii::CommandBuffer& GetCommandBuffer(uint32_t index = 0);
		const vk::raii::CommandBuffer& GetCommandBuffer(uint32_t index = 0) const;

		uint32_t GetCount() const;
		bool IsSingleTime() const;

		void Begin(uint32_t index);
		void End(uint32_t index);
		void Submit(void* waitSemaphore = nullptr, void* signalSemaphore = nullptr, void* fence = nullptr);
		void SubmitAndWait();

		void BindPipeline(void* pipeline);
		void BindVertexBuffer(void* buffer, size_t offset);
		void BindIndexBuffer(void* buffer, size_t offset);
		void BindDescriptorSets(void* pipelineLayout, void* descriptorSet, uint32_t firstSet);
		void SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth);
		void SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height);
		void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
		void BeginRendering(uint32_t width, uint32_t height, void* colorImageView, void* depthImageView);
		void EndRendering();

		void CopyBuffer(void* srcBuffer, void* dstBuffer, size_t size);

		void PushConstants(void* pipelineLayout, uint32_t stageFlags, uint32_t offset, uint32_t size, const void* pValues);

		void TransitionImageLayout(void* image,
			vk::ImageLayout oldLayout,
			vk::ImageLayout newLayout,
			vk::AccessFlags srcAccess,
			vk::AccessFlags dstAccess,
			vk::PipelineStageFlags srcStage,
			vk::PipelineStageFlags dstStage,
			bool isDepth);

		void ResolveImage(void* srcImage, void* dstImage, uint32_t width, uint32_t height);
	};
}

#endif //VRAKTAL_CORE_GPU_VULKAN_COMMANDBUFFER_H