#ifndef VRAKTAL_CORE_GPU_COMMAND_BUFFER_H
#define VRAKTAL_CORE_GPU_COMMAND_BUFFER_H
#pragma once

#include <memory>
#include <cstddef>
#include <core/enum.h>

namespace core::gpu
{
    struct CommandBufferCreateInfo
    {
        void* commandPool = nullptr;
        CommandBufferLevel level = CommandBufferLevel::Primary;
        uint32_t count = 1;
        bool singleTime = false;
    };

    class CommandBuffer
    {
    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;

    public:
        CommandBuffer(void* device, void* queue, const CommandBufferCreateInfo& info);
        ~CommandBuffer();

        CommandBuffer(const CommandBuffer&) = delete;
        CommandBuffer& operator=(const CommandBuffer&) = delete;

        CommandBuffer(CommandBuffer&&) noexcept;
        CommandBuffer& operator=(CommandBuffer&&) noexcept;

        void* GetHandle(uint32_t index = 0) const;
        uint32_t GetCount() const;

        void Begin(uint32_t index = 0);
        void End(uint32_t index = 0);

        void Submit();
        void SubmitAndWait();

        void BindPipeline(void* pipeline);
		void BindVertexBuffer(void* buffer, size_t offset = 0);
		void BindIndexBuffer(void* buffer, size_t offset = 0);
        void BindDescriptorSets(void* pipelineLayout, void* descriptorSet, uint32_t firstSet = 0);

        void SetViewport(float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f);
        void SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height);

        void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1,
            uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0);

        void BeginRendering(uint32_t width, uint32_t height, void* colorImageView, void* depthImageView);
        void EndRendering();

        void TransitionImageLayout(void* image, ImageLayout oldLayout, ImageLayout newLayout);
        void ResolveImage(void* srcImage, void* dstImage, uint32_t width, uint32_t height);

        Impl& GetImpl();
        const Impl& GetImpl() const;
    };
}

#endif //VRAKTAL_CORE_GPU_COMMAND_BUFFER_H