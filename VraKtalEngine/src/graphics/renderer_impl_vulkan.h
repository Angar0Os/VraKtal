#ifndef VRAKTAL_GRAPHICS_RENDERER_IMPL_VULKAN_H
#define VRAKTAL_GRAPHICS_RENDERER_IMPL_VULKAN_H
#pragma once

#include "../core/gpu/commandBuffer_impl_vulkan.h"
#include "../graphics/meshRenderer.h"
#include <core/gpu/renderingInfo.h>

#include "renderGraph.h"

#include <memory>

namespace rhi::core::gpu
{
    class RendererVulkan
    {
    public:
        explicit RendererVulkan(graphics::MeshRenderer* meshRenderer);
        ~RendererVulkan();

        void Render(rhi::vulkan::CommandBufferVulkan& commandBuffer,
            const RenderingInfo& info,
            uint32_t imageIndex,
            const std::vector<graphics::GpuMesh>& meshes,
            const glm::mat4& view,
            const glm::mat4& proj);

    private:
        graphics::MeshRenderer* m_meshRenderer;
        std::unique_ptr<graphics::RenderGraph> m_renderGraph;
    };
};

#endif //VRAKTAL_GRAPHICS_RENDERER_IMPL_VULKAN_H
