#ifndef VRAKTAL_CORE_GPU_COMMANDBUFFER_H
#define VRAKTAL_CORE_GPU_COMMANDBUFFER_H
#pragma once

#include <memory>
#include <vector>
#include <core/enum.h>

namespace core::gpu
{
    class AccelerationStructure;
    class Buffer;
    class DescriptorSet;
    class Device;
    class Image;
    class Pipeline;

    struct SCommandBufferCreateInfo
    {
        const core::gpu::Device* device;
        ECommandBufferLevel level = ECommandBufferLevel::Primary;
        uint32_t count = 1;
        bool singleTime = false;
    };

    class CommandBuffer
    {
    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;

    public:
        CommandBuffer(const core::gpu::Device* _device, const SCommandBufferCreateInfo& _info);
        ~CommandBuffer();

        CommandBuffer(const CommandBuffer&) = delete;
        CommandBuffer& operator=(const CommandBuffer&) = delete;

        CommandBuffer(CommandBuffer&&) noexcept;
        CommandBuffer& operator=(CommandBuffer&&) noexcept;

        uint32_t GetCount() const;

        void Begin(uint32_t _index = 0);
        void End(uint32_t _index = 0);

        void Submit(const core::gpu::Device* _device, uint32_t frameIndex);
        void SubmitAndWait(const core::gpu::Device* _device, uint32_t frameIndex);
        void SubmitImmediate(const core::gpu::Device* device);

        void BindPipeline(const core::gpu::Pipeline* _pipeline);
        void BindVertexBuffer(const core::gpu::Buffer* _buffer, size_t _offset = 0);
        void BindIndexBuffer(const core::gpu::Buffer* _buffer, size_t _offset = 0);
        void BindDescriptorSets(const Pipeline* currentPipeline, const DescriptorSet* descriptorSet, uint32_t _frameIndex, uint32_t _firstSet = 0);

        void SetViewport(float _x, float _y, const core::gpu::Device* device, float _minDepth = 0.0f, float _maxDepth = 1.0f);
        void SetScissor(int32_t _x, int32_t _y, const core::gpu::Device* device);

        void SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth);
        void SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height);

        void DrawIndexed(uint32_t _indexCount, uint32_t _instanceCount = 1,
            uint32_t _firstIndex = 0, int32_t _vertexOffset = 0, uint32_t _firstInstance = 0);

        void BeginRendering(const core::gpu::Device* device, const core::gpu::Image* _colorImageView, const core::gpu::Image* _depthImageView);

        struct RenderingAttachmentInfo
        {
            const core::gpu::Image* image = nullptr;
            bool                    clear = true;
            float                   clearR = 0.0f;
            float                   clearG = 0.0f;
            float                   clearB = 0.0f;
            float                   clearA = 1.0f;
        };
        struct DepthAttachmentInfo
        {
            const core::gpu::Image* image = nullptr;
            bool                    clear = true;
            float                   clearDepth = 1.0f;
        };
        void BeginRendering(const core::gpu::Device* device,
            const std::vector<RenderingAttachmentInfo>& colorAttachments,
            const DepthAttachmentInfo& depthAttachment);

        void EndRendering();

        void TransitionImageLayout(const core::gpu::Image* _image, ImageLayout _oldLayout, ImageLayout _newLayout, bool _isDepth = false);
        void ResolveImage(const core::gpu::Image* _srcImage, const core::gpu::Image* _dstImage, const core::gpu::Device* device);
        void BlitImage(const core::gpu::Image* _srcImage, const core::gpu::Image* _dstImage, const core::gpu::Device* device);

        void CopyBuffer(const core::gpu::Buffer* _srcBuffer, const core::gpu::Buffer* _dstBuffer, size_t _size);
        void CopyBufferToImage(const core::gpu::Buffer* _srcBuffer, const core::gpu::Image* _dstImage, uint32_t width, uint32_t height);

        void PushConstants(const core::gpu::Pipeline* _pipeline, uint32_t _stageFlags, uint32_t _offset, uint32_t _size, const void* _pValues);

        void BuildAccelerationStructure(const core::gpu::AccelerationStructure* _accelerationStructure);

        void AccelerationStructureBarrier();

        void BindRayTracingPipeline(const core::gpu::Pipeline* _pipeline);

        void TraceRays(
            const core::gpu::Device* _device,
            void* _raygenSBT, uint32_t _raygenOffset, uint32_t _raygenStride,
            void* _missSBT, uint32_t _missOffset, uint32_t _missStride, uint32_t _missCount,
            void* _hitSBT, uint32_t _hitOffset, uint32_t _hitStride, uint32_t _hitCount,
            void* _callableSBT, uint32_t _callableOffset, uint32_t _callableStride, uint32_t _callableCount,
            uint32_t _width, uint32_t _height, uint32_t _depth);

        Impl& GetImpl() const;
    };
}

#endif //VRAKTAL_CORE_GPU_COMMANDBUFFER_H