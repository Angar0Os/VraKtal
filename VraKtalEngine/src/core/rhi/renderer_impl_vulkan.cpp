#include "../src/core/rhi/renderer_impl_vulkan.h"
#include "../src/core/rhi/commandBuffer_impl_vulkan.h"

using namespace core::rhi::vulkan;
using namespace core::graphics;

RendererVulkan::RendererVulkan(MeshRenderer* meshRenderer)
    : m_meshRenderer(meshRenderer)
{
}

void RendererVulkan::Render(CommandBufferVulkan& commandBuffer,
    const RenderingInfo& info,
    uint32_t imageIndex,
    const std::vector<GpuMesh>& meshes,
    const glm::mat4& view,
    const glm::mat4& proj)
{
    commandBuffer.BeginRendering(info, imageIndex);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)info.width;
    viewport.height = (float)info.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    VkExtent2D infoExtent{ info.width, info.height };
    scissor.offset = { 0, 0 };
    scissor.extent = infoExtent;

    vkCmdSetViewport(commandBuffer.GetNative(), 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer.GetNative(), 0, 1, &scissor);

    for (auto& mesh : meshes)
    {
        glm::mat4 model = glm::mat4(1.0f);
        m_meshRenderer->Draw(commandBuffer, mesh, model, view, proj);
    }

    commandBuffer.EndRendering(imageIndex);
}
