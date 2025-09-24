#ifndef VRAKTAL_GRAPHICS_MESH_RENDERER_H
#define VRAKTAL_GRAPHICS_MESH_RENDERER_H
#pragma once

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <glm/glm.hpp>
#include <vector>

#include "../src/core/gpu/gpuDevice_impl_glfw_vulkan.h"

namespace rhi::vulkan { class CommandBufferVulkan; }
namespace graphics::resources { struct Vertex; struct Material; class Mesh; }

namespace graphics
{
    struct GpuMesh
    {
        VkBuffer vertexBuffer = VK_NULL_HANDLE;
        VmaAllocation vertexAlloc = VK_NULL_HANDLE;
        VkBuffer indexBuffer = VK_NULL_HANDLE;
        VmaAllocation indexAlloc = VK_NULL_HANDLE;
        uint32_t indexCount = 0;
        int materialIndex = -1;
    };

    class MeshRenderer
    {
    public:
        MeshRenderer(rhi::vulkan::GpuDeviceVulkan& device);
        ~MeshRenderer();

        GpuMesh UploadMesh(const resources::Mesh& mesh);
        void DestroyMesh(GpuMesh& mesh);

        void Draw(rhi::vulkan::CommandBufferVulkan& cmd, const GpuMesh& mesh,
                 const glm::mat4& model = glm::mat4(1.0f),
                 const glm::mat4& view = glm::mat4(1.0f),
                 const glm::mat4& projection = glm::mat4(1.0f));

        VkDescriptorSetLayout DescriptorSetLayout() const { return m_descriptorSetLayout; }
        rhi::vulkan::GpuDeviceVulkan GetGpuDevice() const { return m_device; }

        VkDescriptorSet CreateDescriptorSet(VkImageView view, VkSampler sampler);

        void CreateDescriptorPool(uint32_t maxSets);

        void SetMaterialDescriptorSets(const std::vector<VkDescriptorSet>& sets);
        void CreateDescriptorSetLayout();

    private:
        void CreatePipeline();
        void DestroyDescriptors();

        rhi::vulkan::GpuDeviceVulkan& m_device;
        VkPipeline          m_pipeline = VK_NULL_HANDLE;
        VkPipelineLayout    m_pipelineLayout = VK_NULL_HANDLE;

        VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorPool      m_descriptorPool = VK_NULL_HANDLE;

        std::vector<VkDescriptorSet> m_materialDescriptorSets;
    };
}

#endif //VRAKTAL_GRAPHICS_MESH_RENDERER_H
