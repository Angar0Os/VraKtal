#include <graphics/renderer.h>

#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <iostream>

using namespace graphics;


Renderer::Renderer(core::Window& window, core::gpu::Device& device)
    : m_window(window),
    m_device(device),
    m_currentFrame(0),
    m_running(true),
    m_frameCounter(0),
    m_viewMatrix(glm::mat4(1.0f)),
    m_projMatrix(glm::mat4(1.0f)),
    m_cameraPosition(glm::vec3(0.0f))
{
    CreateCommandBuffers();
}

Renderer::~Renderer()
{
    Cleanup();
}

void Renderer::CreateCommandBuffers()
{
    m_commandBuffers.clear();
    m_commandBuffers.reserve(core::gpu::Device::FRAMES_IN_FLIGHT);

    for (uint32_t i = 0; i < core::gpu::Device::FRAMES_IN_FLIGHT; i++)
    {
        core::gpu::CommandBufferCreateInfo cmdInfo{};
        cmdInfo.commandPool = m_device.GetCommandPool();
        cmdInfo.level = core::CommandBufferLevel::Primary;
        cmdInfo.count = 1;

        auto cmdBuffer = std::make_unique<core::gpu::CommandBuffer>(
            m_device.GetHandle(),
            m_device.GetGraphicsQueue(),
            cmdInfo
        );

        m_commandBuffers.push_back(std::move(cmdBuffer));
    }
}

void Renderer::SetScene(std::shared_ptr<resources::Scene> scene)
{
    m_scene = scene;

    if (m_scene)
    {
        for (const auto& instance : m_scene->meshInstances)
        {
            if (m_meshBuffers.find(instance.mesh.get()) == m_meshBuffers.end())
            {
                CreateMeshBuffers(instance.mesh);
            }
        }
    }
}

void Renderer::CreateMeshBuffers(std::shared_ptr<resources::Mesh> mesh)
{
    if (!mesh || mesh->vertices.empty() || mesh->indices.empty())
    {
        std::cerr << "ERROR: Invalid mesh data!" << std::endl;
        return;
    }

    MeshBuffers buffers;

    size_t vertexBufferSize = mesh->vertices.size() * sizeof(resources::Vertex);
    size_t indexBufferSize = mesh->indices.size() * sizeof(uint32_t);

    core::gpu::BufferCreateInfo stagingVertexInfo{
        .size = vertexBufferSize,
        .usage = core::BufferUsage::TransferSrc,
        .memoryProperties = core::MemoryProperty::HostVisible | core::MemoryProperty::HostCoherent
    };

    auto stagingVertexBuffer = std::make_unique<core::gpu::Buffer>(
        m_device.GetHandle(),
        m_device.GetPhysicalDevice(),
        stagingVertexInfo
    );

    core::gpu::BufferCreateInfo stagingIndexInfo{
        .size = indexBufferSize,
        .usage = core::BufferUsage::TransferSrc,
        .memoryProperties = core::MemoryProperty::HostVisible | core::MemoryProperty::HostCoherent
    };

    auto stagingIndexBuffer = std::make_unique<core::gpu::Buffer>(
        m_device.GetHandle(),
        m_device.GetPhysicalDevice(),
        stagingIndexInfo
    );

    stagingVertexBuffer->CopyFrom(mesh->vertices.data(), vertexBufferSize);
    stagingIndexBuffer->CopyFrom(mesh->indices.data(), indexBufferSize);

    core::gpu::BufferCreateInfo vertexInfo{
        .size = vertexBufferSize,
        .usage = core::BufferUsage::VertexBuffer | core::BufferUsage::TransferDst,
        .memoryProperties = core::MemoryProperty::DeviceLocal
    };

    buffers.vertexBuffer = std::make_unique<core::gpu::Buffer>(
        m_device.GetHandle(),
        m_device.GetPhysicalDevice(),
        vertexInfo
    );

    core::gpu::BufferCreateInfo indexInfo{
        .size = indexBufferSize,
        .usage = core::BufferUsage::IndexBuffer | core::BufferUsage::TransferDst,
        .memoryProperties = core::MemoryProperty::DeviceLocal
    };

    buffers.indexBuffer = std::make_unique<core::gpu::Buffer>(
        m_device.GetHandle(),
        m_device.GetPhysicalDevice(),
        indexInfo
    );

    buffers.indexCount = static_cast<uint32_t>(mesh->indices.size());

    core::gpu::CommandBufferCreateInfo cmdInfo{};
    cmdInfo.commandPool = m_device.GetCommandPool();
    cmdInfo.level = core::CommandBufferLevel::Primary;
    cmdInfo.count = 1;
    cmdInfo.singleTime = true;

    auto transferCmd = std::make_unique<core::gpu::CommandBuffer>(
        m_device.GetHandle(),
        m_device.GetGraphicsQueue(),
        cmdInfo
    );

    transferCmd->Begin(0);
    transferCmd->CopyBuffer(
        stagingVertexBuffer->GetHandle(),
        buffers.vertexBuffer->GetHandle(),
        vertexBufferSize
    );
    transferCmd->CopyBuffer(
        stagingIndexBuffer->GetHandle(),
        buffers.indexBuffer->GetHandle(),
        indexBufferSize
    );
    transferCmd->End(0);

    transferCmd->SubmitAndWait();

    m_meshBuffers[mesh.get()] = std::move(buffers);
}

void Renderer::UpdateCamera(const glm::mat4& view, const glm::mat4& proj, const glm::vec3& position)
{
    m_viewMatrix = view;
    m_projMatrix = proj;
    m_cameraPosition = position;
}

void Renderer::UpdateUniformBuffer(uint32_t frameIndex)
{
    core::gpu::UniformBufferObject ubo{};

    float angle = (m_frameCounter % 360) * 3.14159f / 180.0f;
    ubo.model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f));

    ubo.view = m_viewMatrix;
    ubo.proj = m_projMatrix;
    ubo.viewPos = m_cameraPosition;

    auto* uniformBuffer = m_device.GetUniformBuffer(frameIndex);
    if (uniformBuffer)
    {
        uniformBuffer->CopyFrom(&ubo, sizeof(core::gpu::UniformBufferObject));
    }
}

void Renderer::RecordCommandBuffer(uint32_t frameIndex, uint32_t imageIndex)
{
    auto& cmd = m_commandBuffers[frameIndex];

    cmd->Begin(0);

    uint32_t width = m_device.GetSwapchainWidth();
    uint32_t height = m_device.GetSwapchainHeight();

    void* colorImageHandle = m_device.GetColorImage();
    void* swapchainImageHandle = m_device.GetSwapchainImage(imageIndex);
    void* depthImageHandle = m_device.GetDepthImage();

    cmd->TransitionImageLayout(
        colorImageHandle,
        core::ImageLayout::Undefined,
        core::ImageLayout::ColorAttachment,
        false 
    );

    cmd->TransitionImageLayout(
        swapchainImageHandle,
        core::ImageLayout::Undefined,
        core::ImageLayout::TransferDst,
        false 
    );

    cmd->TransitionImageLayout(
        depthImageHandle,
        core::ImageLayout::Undefined,
        core::ImageLayout::Undefined,  
        true  
    );

    cmd->BeginRendering(
        width, height,
        m_device.GetColorImageView(),
        m_device.GetDepthImageView()
    );

    cmd->BindPipeline(m_device.GetPipeline());
    cmd->SetViewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    cmd->SetScissor(0, 0, width, height);
    cmd->BindDescriptorSets(
        m_device.GetPipelineLayout(),
        m_device.GetDescriptorSet(frameIndex),
        0
    );

    if (m_scene)
    {
        for (const auto& instance : m_scene->meshInstances)
        {
            auto it = m_meshBuffers.find(instance.mesh.get());
            if (it != m_meshBuffers.end())
            {
                const auto& buffers = it->second;

                cmd->BindVertexBuffer(buffers.vertexBuffer->GetHandle());
                cmd->BindIndexBuffer(buffers.indexBuffer->GetHandle());
                cmd->DrawIndexed(buffers.indexCount);
            }
            else
            {
                std::cout << "ERROR: Mesh buffers not found!" << std::endl;
            }
        }
    }
    else
    {
        std::cout << "ERROR: No scene set!" << std::endl;
    }

    cmd->EndRendering();

    cmd->TransitionImageLayout(
        colorImageHandle,
        core::ImageLayout::ColorAttachment,
        core::ImageLayout::TransferSrc,
        false
    );

    cmd->ResolveImage(
        colorImageHandle,
        swapchainImageHandle,
        width,
        height
    );

    cmd->TransitionImageLayout(
        swapchainImageHandle,
        core::ImageLayout::TransferDst,
        core::ImageLayout::Present,
        false
    );

    cmd->End(0);
}

void Renderer::DrawFrame()
{
    if (!m_running) return;

    m_device.BeginFrame(m_currentFrame);

    uint32_t imageIndex = m_device.AcquireNextImage(m_currentFrame);
    if (imageIndex == UINT32_MAX)
    {
        return;
    }

    UpdateUniformBuffer(m_currentFrame);
    RecordCommandBuffer(m_currentFrame, imageIndex);

    void* waitSemaphore = m_device.GetImageAvailableSemaphore(m_currentFrame);
    void* signalSemaphore = m_device.GetRenderFinishedSemaphore(imageIndex);
    void* fence = m_device.GetInFlightFence(m_currentFrame);

    m_commandBuffers[m_currentFrame]->Submit(waitSemaphore, signalSemaphore, fence);

    m_device.Present(imageIndex);

    m_currentFrame = (m_currentFrame + 1) % core::gpu::Device::FRAMES_IN_FLIGHT;
    m_frameCounter++;
}

void Renderer::Cleanup()
{
    m_device.WaitIdle();

    if (!m_running) return;
    m_running = false;

    m_meshBuffers.clear();
    m_commandBuffers.clear();
    m_device.Cleanup();
}