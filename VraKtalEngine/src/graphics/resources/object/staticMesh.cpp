#include <graphics/resources/object/staticMesh.h>

using namespace graphics::resources::object;

void StaticMesh::Render(graphics::Renderer& renderer)
{
    auto& cmd = renderer.m_commandBuffers[renderer.m_currentFrame];

    const resources::object::Material* lastMaterial = nullptr;

    if(!this->visible) return;
    if(!mesh || !material) return;

    if(material.get() != lastMaterial)
    {
        renderer.UpdateUniformBuffer(renderer.m_currentFrame, material);

        cmd->BindDescriptorSets(
            &renderer.m_device,
            renderer.m_currentFrame,
            0
        );
        lastMaterial = material.get();
    }

    PushConstants pushConstants;
    pushConstants.model = GetTransformMatrix();

    cmd->PushConstants(
        renderer.m_device.GetGraphicsPipeline(),
        static_cast<uint32_t>(core::ShaderStageFlags::Vertex),
        0,
        sizeof(PushConstants),
        &pushConstants
    );

    auto it = renderer.m_meshBuffers.find(mesh.get());
    if(it != renderer.m_meshBuffers.end())
    {
        const auto& buffers = it->second;
        cmd->BindVertexBuffer(buffers.vertexBuffer.get());
        cmd->BindIndexBuffer(buffers.indexBuffer.get());
        cmd->DrawIndexed(buffers.indexCount);
    }
}