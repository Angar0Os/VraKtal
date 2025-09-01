#include "meshRenderer.h"

#include <stdexcept>

#include "../core/gpu/commandBuffer_impl_vulkan.h"

using namespace rhi::core::gpu;
using namespace rhi::vulkan;

graphics::MeshRenderer::MeshRenderer(GpuDeviceVulkan& device)
    : m_device(device)
{
}

graphics::MeshRenderer::~MeshRenderer()
{
    VkDevice dev = m_device.Device();
    if (m_pipeline)
        {
        vkDestroyPipeline(dev, m_pipeline, nullptr);
    }
    if (m_pipelineLayout)
        {
        vkDestroyPipelineLayout(dev, m_pipelineLayout, nullptr);
    }
}

graphics::GpuMesh graphics::MeshRenderer::UploadMesh(const Mesh& mesh)
{
    GpuMesh gpuMesh{};

    VkDeviceSize vertexBufferSize = mesh.vertices.size() * sizeof(Vertex);
    VkDeviceSize indexBufferSize  = mesh.indices.size() * sizeof(uint32_t);

    {
        VkBufferCreateInfo vbInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        vbInfo.size  = vertexBufferSize;
        vbInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        if (vmaCreateBuffer(m_device.Allocator(), &vbInfo, &allocInfo,
                            &gpuMesh.vertexBuffer, &gpuMesh.vertexAlloc, nullptr) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create vertex buffer");
        }

        m_device.UploadToBuffer(gpuMesh.vertexBuffer, mesh.vertices.data(), vertexBufferSize);
    }

    {
        VkBufferCreateInfo ibInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        ibInfo.size  = indexBufferSize;
        ibInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        if (vmaCreateBuffer(m_device.Allocator(), &ibInfo, &allocInfo,
                            &gpuMesh.indexBuffer, &gpuMesh.indexAlloc, nullptr) != VK_SUCCESS)
            {
            throw std::runtime_error("Failed to create index buffer");
        }

        m_device.UploadToBuffer(gpuMesh.indexBuffer, mesh.indices.data(), indexBufferSize);
    }

    gpuMesh.indexCount = static_cast<uint32_t>(mesh.indices.size());
    return gpuMesh;
}

void graphics::MeshRenderer::DestroyMesh(GpuMesh& mesh)
{
    if (mesh.vertexBuffer)
    {
        vmaDestroyBuffer(m_device.Allocator(), mesh.vertexBuffer, mesh.vertexAlloc);
        mesh.vertexBuffer = VK_NULL_HANDLE;
    }
    if (mesh.indexBuffer)
    {
        vmaDestroyBuffer(m_device.Allocator(), mesh.indexBuffer, mesh.indexAlloc);
        mesh.indexBuffer = VK_NULL_HANDLE;
    }
}

void graphics::MeshRenderer::Draw(CommandBufferVulkan& cmd, const GpuMesh& mesh)
{
    auto& vkCmd = reinterpret_cast<CommandBufferVulkan&>(cmd);

    VkCommandBuffer nativeCmd = vkCmd.GetNative();

    VkDeviceSize offsets[] = { 0 };
    VkBuffer vertexBuffers[] = { mesh.vertexBuffer };

    vkCmdBindPipeline(nativeCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

    vkCmdBindVertexBuffers(nativeCmd, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(nativeCmd, mesh.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(nativeCmd, mesh.indexCount, 1, 0, 0, 0);
}