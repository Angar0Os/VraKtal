#include "renderer_impl_vulkan.h"
#include "meshRenderer.h"
#include "renderGraph.h"

using namespace rhi::core::gpu;
using namespace rhi::vulkan;
using namespace graphics;

RendererVulkan::RendererVulkan(MeshRenderer* meshRenderer)
    : m_meshRenderer(meshRenderer)
{
    m_renderGraph = std::make_unique<RenderGraph>(meshRenderer->GetGpuDevice().Device(), meshRenderer->GetGpuDevice().Allocator(), 1920, 1080);
}

RendererVulkan::~RendererVulkan()
{
    if (m_renderGraph) 
    {
        m_renderGraph->Cleanup();
        m_renderGraph.reset();
    }
}

void RendererVulkan::Render(CommandBufferVulkan& commandBuffer,
    const RenderingInfo& info,
    uint32_t imageIndex,
    const std::vector<GpuMesh>& meshes,
    const glm::mat4& view,
    const glm::mat4& proj)
{
    if (m_renderGraph->Width() != info.width || m_renderGraph->Height() != info.height)
    {
        m_renderGraph->Resize(info.width, info.height);
    }

    m_renderGraph->Clear();

    RGTextureDesc colorDesc{};
    colorDesc.width = info.width;
    colorDesc.height = info.height;
    colorDesc.format = m_meshRenderer->GetGpuDevice().SwapFormat();
    colorDesc.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    m_renderGraph->AddTexture("color", colorDesc);

    RGTextureDesc depthDesc{};
    depthDesc.width = info.width;
    depthDesc.height = info.height;
    depthDesc.format = m_meshRenderer->GetGpuDevice().DepthFormat();
    depthDesc.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    m_renderGraph->AddTexture("depth", depthDesc);

    RenderPassNode meshPass{};
    meshPass.name = "MeshPass";

    meshPass.execute = [&](VkCommandBuffer vkCmd, const RenderGraphResources& resources)
        {
            RGTexture* colorTex = resources.textures.at("color");
            RGTexture* depthTex = resources.textures.at("depth");

            VkRenderingAttachmentInfo colorAttach{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
            colorAttach.imageView = colorTex->view;
            colorAttach.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorAttach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            VkClearValue clearColor{};
            clearColor.color = { {0.0f, 0.0f, 0.0f, 1.0f} };
            colorAttach.clearValue = clearColor;

            VkRenderingAttachmentInfo depthAttach{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
            depthAttach.imageView = depthTex->view;
            depthAttach.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthAttach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttach.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            VkClearValue clearDepth{};
            clearDepth.depthStencil = { 1.0f, 0 };
            depthAttach.clearValue = clearDepth;

            VkRenderingInfo renderingInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
            renderingInfo.renderArea.offset = { 0, 0 };
            renderingInfo.renderArea.extent = { info.width, info.height };
            renderingInfo.layerCount = 1;
            renderingInfo.colorAttachmentCount = 1;
            renderingInfo.pColorAttachments = &colorAttach;
            renderingInfo.pDepthAttachment = &depthAttach;

            vkCmdBeginRendering(vkCmd, &renderingInfo);

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

            vkCmdEndRendering(vkCmd);
        };

    m_renderGraph->AddPass(meshPass);

    m_renderGraph->Compile();
    m_renderGraph->Execute(commandBuffer.GetNative());
}
