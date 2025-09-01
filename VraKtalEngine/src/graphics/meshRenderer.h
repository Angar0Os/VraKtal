#ifndef VRAKTAL_GRAPHICS_MESH_RENDERER_H
#define VRAKTAL_GRAPHICS_MESH_RENDERER_H
#pragma once

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

#include "resources/mesh.h"
#include "../core/gpu/gpuDevice_impl_glfw_vulkan.h"

using namespace graphics::resources;

namespace graphics
{
    struct GpuMesh
    {
        VkBuffer vertexBuffer = VK_NULL_HANDLE;
        VmaAllocation vertexAlloc = VK_NULL_HANDLE;
        VkBuffer indexBuffer = VK_NULL_HANDLE;
        VmaAllocation indexAlloc = VK_NULL_HANDLE;
        uint32_t indexCount = 0;
    };

    class MeshRenderer
    {
    public:
        MeshRenderer(rhi::vulkan::GpuDeviceVulkan& device);
        ~MeshRenderer();

        GpuMesh UploadMesh(const Mesh& mesh);
        void DestroyMesh(GpuMesh& mesh);

        void Draw(rhi::vulkan::CommandBufferVulkan& cmd, const GpuMesh& mesh);

    private:
        rhi::vulkan::GpuDeviceVulkan& m_device;

        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_pipeline = VK_NULL_HANDLE;
    };
}

#endif //VRAKTAL_GRAPHICS_MESH_RENDERER_H
