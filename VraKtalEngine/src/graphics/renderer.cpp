#include <graphics/renderer.h>

#include <iostream>
#include <thread>
#include <chrono>
#include <stdexcept>

using namespace graphics;

Renderer::Renderer(core::Window& window, core::gpu::Device& device)
    : m_window(window),
    m_device(device),
    m_frameManager(device),
    m_frameCallback(nullptr),
    m_running(true),
    m_currentFrame(0)
{
}

Renderer::~Renderer()
{
    Cleanup();
}

void Renderer::SetFrameCallback(FrameCallback cb)
{
    m_frameCallback = std::move(cb);
}

void Renderer::DrawFrame()
{
    if (!m_running.load()) return;

    GLFWwindow* win = m_window.GlfwHandle();
    if (!win) return;

    m_frameManager.BeginFrame(m_currentFrame);

    uint32_t imageIndex = m_frameManager.AcquireNextImage(m_currentFrame);
    if (imageIndex == UINT32_MAX) return;

    void* imageAvailable = m_frameManager.GetImageAvailableSemaphore(m_currentFrame);
    void* renderFinished = m_frameManager.GetRenderFinishedSemaphore(imageIndex);
    void* inFlightFence = m_frameManager.GetInFlightFence(m_currentFrame);

    bool submittedByCallback = false;
    if (m_frameCallback)
    {
        submittedByCallback = m_frameCallback(imageIndex, imageAvailable, renderFinished, inFlightFence);
    }

    if (!submittedByCallback)
    {
        m_frameManager.SubmitDefaultTransitionIfNeeded(m_currentFrame, imageIndex);
    }

    m_frameManager.Present(imageIndex);

    m_currentFrame = (m_currentFrame + 1) % core::gpu::FrameManager::FRAMES_IN_FLIGHT;
}

void Renderer::Cleanup()
{
    if (!m_running.exchange(false)) return;
    m_frameManager.Cleanup();
}