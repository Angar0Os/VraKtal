#include "../src/core/gpu/window_impl_vulkan.h"
#include "../src/core/gpu/gpuDevice_impl_glfw_vulkan.h"
#include "../src/core/gpu/commandBuffer_impl_vulkan.h"
#include "../src/core/gpu/createTrianglePipeline_impl_vulkan.h"
#include "../src/core/gpu/renderer_impl_vulkan.h"

#include <core/gpu/renderingInfo.h>
#include <vector>

#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "vulkan-1.lib")

using namespace rhi::vulkan;
using namespace rhi::core::gpu;

int main()
{
    WindowVulkan window(800, 600, "Triangle");
    GpuDeviceVulkan device(window);

    device.WrapSwapchainImages();

    Pipeline* pipeline = CreateTrianglePipeline(device);
    TriangleRenderer triangleRenderer(pipeline);

    std::vector<CommandBufferVulkan*> commandBuffers;
    for (int i = 0; i < 2; ++i)
    {
        commandBuffers.push_back(new CommandBufferVulkan(device));
    }

    uint32_t currentFrame = 0;

    while (!window.ShouldClose())
    {
        window.PollEvents();

        uint32_t imageIndex;
        if (!device.BeginFrame(imageIndex))
        {
            continue;
        }

        CommandBufferVulkan& commandBuffer = *commandBuffers[currentFrame];

        commandBuffer.Begin();

        RenderingInfo info;
        info.width = device.SwapExtent().width;
        info.height = device.SwapExtent().height;

        RenderingAttachment color{};
        color.image = device.GetSwapchainImage(imageIndex);
        color.clearValue = { 0.1f, 0.1f, 0.15f, 1.f };
        color.loadOp = LoadOp::Clear;
        color.storeOp = StoreOp::Store;
        info.colorAttachments.push_back(color);

        triangleRenderer.Render(commandBuffer, info, imageIndex);

        commandBuffer.End();
        device.EndFrame(imageIndex, commandBuffer.GetNative());

        currentFrame = (currentFrame + 1) % 2;
    }

    device.WaitIdle();
    
    for (auto* cmd : commandBuffers)
    {
        delete cmd;
    }
    delete pipeline;

    return 0;
}