#include <core/gpu/device.h>
#include <iostream>
#include <stdexcept>

#include "../src/core/gpu/vulkan/frameManager_impl.h"

using namespace core::gpu;

FrameManager::Impl::Impl(Device& dev)
    : device(dev)
{
    vkDevice = reinterpret_cast<VkDevice>(device.GetDeviceHandle());
    vkQueue = reinterpret_cast<VkQueue>(device.GetGraphicsQueueHandle());
    vkCommandPool = reinterpret_cast<VkCommandPool>(device.GetCommandPoolHandle());

    if (!vkDevice) throw std::runtime_error("FrameManager: invalid VkDevice");

    imageAvailable.assign(FrameManager::FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
    inFlightFences.assign(FrameManager::FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
    tempCmdBufs.assign(FrameManager::FRAMES_IN_FLIGHT, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t i = 0; i < FrameManager::FRAMES_IN_FLIGHT; ++i)
    {
        if (vkCreateSemaphore(vkDevice, &semInfo, nullptr, &imageAvailable[i]) != VK_SUCCESS)
            throw std::runtime_error("FrameManager: failed to create imageAvailable semaphore");
        if (vkCreateFence(vkDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
            throw std::runtime_error("FrameManager: failed to create inFlight fence");
    }

    swapchainImageCount = device.GetSwapchainImageCount();
    renderFinished.assign(swapchainImageCount, VK_NULL_HANDLE);
    imagesInFlight.assign(swapchainImageCount, VK_NULL_HANDLE);

    for (uint32_t i = 0; i < swapchainImageCount; ++i)
    {
        if (vkCreateSemaphore(vkDevice, &semInfo, nullptr, &renderFinished[i]) != VK_SUCCESS)
            throw std::runtime_error("FrameManager: failed to create renderFinished semaphore");
    }
}

FrameManager::Impl::~Impl()
{
    try { Cleanup(); }
    catch (...) {}
}

void FrameManager::Impl::Cleanup()
{
    if (!vkDevice) return;
    vkDeviceWaitIdle(vkDevice);

    for (auto s : imageAvailable) if (s) vkDestroySemaphore(vkDevice, s, nullptr);
    for (auto s : renderFinished) if (s) vkDestroySemaphore(vkDevice, s, nullptr);
    for (auto f : inFlightFences) if (f) vkDestroyFence(vkDevice, f, nullptr);

    if (vkCommandPool != VK_NULL_HANDLE)
    {
        for (auto cb : tempCmdBufs)
            if (cb != VK_NULL_HANDLE)
                vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &cb);
    }

    imageAvailable.clear();
    renderFinished.clear();
    inFlightFences.clear();
    imagesInFlight.clear();
    tempCmdBufs.clear();

    vkDevice = VK_NULL_HANDLE;
    vkQueue = VK_NULL_HANDLE;
    vkCommandPool = VK_NULL_HANDLE;
}

void FrameManager::Impl::BeginFrame(uint32_t frameIndex)
{
    VkFence fence = inFlightFences[frameIndex];
    vkWaitForFences(vkDevice, 1, &fence, VK_TRUE, UINT64_MAX);

    if (tempCmdBufs[frameIndex] != VK_NULL_HANDLE && vkCommandPool != VK_NULL_HANDLE)
    {
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &tempCmdBufs[frameIndex]);
        tempCmdBufs[frameIndex] = VK_NULL_HANDLE;
    }

    vkResetFences(vkDevice, 1, &fence);
}

uint32_t FrameManager::Impl::AcquireNextImage(uint32_t frameIndex)
{
    VkSwapchainKHR swapchain = reinterpret_cast<VkSwapchainKHR>(device.GetSwapchainHandle());
    if (!swapchain) return UINT32_MAX;

    uint32_t imageIndex = UINT32_MAX;
    VkResult res = vkAcquireNextImageKHR(vkDevice, swapchain, UINT64_MAX, imageAvailable[frameIndex], VK_NULL_HANDLE, &imageIndex);
    if (res == VK_ERROR_OUT_OF_DATE_KHR) return UINT32_MAX;
    if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
    {
        std::cerr << "FrameManager: vkAcquireNextImageKHR failed: " << res << "\n";
        return UINT32_MAX;
    }
    if (imageIndex < imagesInFlight.size() && imagesInFlight[imageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(vkDevice, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight[imageIndex] = inFlightFences[frameIndex];
    return imageIndex;
}

void* FrameManager::Impl::GetImageAvailableSemaphore(uint32_t frameIndex) const
{
    if (frameIndex >= imageAvailable.size()) return nullptr;
    return reinterpret_cast<void*>(imageAvailable[frameIndex]);
}

void* FrameManager::Impl::GetRenderFinishedSemaphore(uint32_t imageIndex) const
{
    if (imageIndex >= renderFinished.size()) return nullptr;
    return reinterpret_cast<void*>(renderFinished[imageIndex]);
}

void* FrameManager::Impl::GetInFlightFence(uint32_t frameIndex) const
{
    if (frameIndex >= inFlightFences.size()) return nullptr;
    return reinterpret_cast<void*>(inFlightFences[frameIndex]);
}

void FrameManager::Impl::SubmitDefaultTransitionIfNeeded(uint32_t frameIndex, uint32_t imageIndex)
{
    VkQueue queue = reinterpret_cast<VkQueue>(device.GetGraphicsQueueHandle());
    VkCommandPool cmdPool = reinterpret_cast<VkCommandPool>(device.GetCommandPoolHandle());
    VkSemaphore imgAvail = imageAvailable[frameIndex];
    VkSemaphore renderFin = (imageIndex < renderFinished.size()) ? renderFinished[imageIndex] : VK_NULL_HANDLE;
    VkFence fence = inFlightFences[frameIndex];

    if (!queue || !cmdPool)
    {
        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &imgAvail;
        submitInfo.pWaitDstStageMask = &waitStage;
        submitInfo.commandBufferCount = 0;
        submitInfo.pCommandBuffers = nullptr;
        submitInfo.signalSemaphoreCount = (renderFin != VK_NULL_HANDLE) ? 1 : 0;
        submitInfo.pSignalSemaphores = (renderFin != VK_NULL_HANDLE) ? &renderFin : nullptr;
        vkQueueSubmit(queue, 1, &submitInfo, fence);
        return;
    }

    void* imgHandle = device.GetSwapchainImageHandle(imageIndex);
    VkImage swapImage = reinterpret_cast<VkImage>(imgHandle);
    if (swapImage == VK_NULL_HANDLE)
    {
        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &imgAvail;
        submitInfo.pWaitDstStageMask = &waitStage;
        submitInfo.commandBufferCount = 0;
        submitInfo.pCommandBuffers = nullptr;
        submitInfo.signalSemaphoreCount = (renderFin != VK_NULL_HANDLE) ? 1 : 0;
        submitInfo.pSignalSemaphores = (renderFin != VK_NULL_HANDLE) ? &renderFin : nullptr;
        vkQueueSubmit(queue, 1, &submitInfo, fence);
        return;
    }

    VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocInfo.commandPool = cmdPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer cmdBuf = VK_NULL_HANDLE;
    if (vkAllocateCommandBuffers(vkDevice, &allocInfo, &cmdBuf) != VK_SUCCESS)
    {
        std::cerr << "FrameManager: failed to allocate temporary command buffer\n";
        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &imgAvail;
        submitInfo.pWaitDstStageMask = &waitStage;
        submitInfo.commandBufferCount = 0;
        submitInfo.pCommandBuffers = nullptr;
        submitInfo.signalSemaphoreCount = (renderFin != VK_NULL_HANDLE) ? 1 : 0;
        submitInfo.pSignalSemaphores = (renderFin != VK_NULL_HANDLE) ? &renderFin : nullptr;
        vkQueueSubmit(queue, 1, &submitInfo, fence);
        return;
    }

    VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmdBuf, &beginInfo);

    VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = swapImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(
        cmdBuf,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    vkEndCommandBuffer(cmdBuf);

    tempCmdBufs[frameIndex] = cmdBuf;

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &imgAvail;
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuf;
    submitInfo.signalSemaphoreCount = (renderFin != VK_NULL_HANDLE) ? 1 : 0;
    submitInfo.pSignalSemaphores = (renderFin != VK_NULL_HANDLE) ? &renderFin : nullptr;

    if (vkQueueSubmit(vkQueue, 1, &submitInfo, inFlightFences[frameIndex]) != VK_SUCCESS)
    {
        std::cerr << "FrameManager: vkQueueSubmit(temp) failed\n";
    }
}

void FrameManager::Impl::Present(uint32_t imageIndex)
{
    VkQueue queue = reinterpret_cast<VkQueue>(device.GetGraphicsQueueHandle());
    VkSwapchainKHR swapchain = reinterpret_cast<VkSwapchainKHR>(device.GetSwapchainHandle());
    if (!queue || !swapchain) return;

    VkSemaphore presentWait = (imageIndex < renderFinished.size()) ? renderFinished[imageIndex] : VK_NULL_HANDLE;
    VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    presentInfo.waitSemaphoreCount = (presentWait != VK_NULL_HANDLE) ? 1 : 0;
    presentInfo.pWaitSemaphores = (presentWait != VK_NULL_HANDLE) ? &presentWait : nullptr;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(queue, &presentInfo);
}

core::gpu::FrameManager::FrameManager(Device& device)
{
    m_impl = new Impl(device);
}

core::gpu::FrameManager::~FrameManager()
{
    if (m_impl)
    {
        delete m_impl;
        m_impl = nullptr;
    }
}

uint32_t core::gpu::FrameManager::AcquireNextImage(uint32_t frameIndex)
{
    if (!m_impl) return UINT32_MAX;
    return m_impl->AcquireNextImage(frameIndex);
}

void core::gpu::FrameManager::BeginFrame(uint32_t frameIndex)
{
    if (!m_impl) return;
    m_impl->BeginFrame(frameIndex);
}

void* core::gpu::FrameManager::GetImageAvailableSemaphore(uint32_t frameIndex) const
{
    if (!m_impl) return nullptr;
    return m_impl->GetImageAvailableSemaphore(frameIndex);
}

void* core::gpu::FrameManager::GetRenderFinishedSemaphore(uint32_t imageIndex) const 
{
    if (!m_impl) return nullptr;
    return m_impl->GetRenderFinishedSemaphore(imageIndex);
}

void* core::gpu::FrameManager::GetInFlightFence(uint32_t frameIndex) const 
{
    if (!m_impl) return nullptr;
    return m_impl->GetInFlightFence(frameIndex);
}

void core::gpu::FrameManager::SubmitDefaultTransitionIfNeeded(uint32_t frameIndex, uint32_t imageIndex)
{
    if (!m_impl) return;
    m_impl->SubmitDefaultTransitionIfNeeded(frameIndex, imageIndex);
}

void core::gpu::FrameManager::Present(uint32_t imageIndex)
{
    if (!m_impl) return;
    m_impl->Present(imageIndex);
}

void core::gpu::FrameManager::Cleanup()
{
    if (!m_impl) return;
    m_impl->Cleanup();
}