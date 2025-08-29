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

    CommandBufferVulkan commandBuffer(device);

    while (!window.ShouldClose())
    {
        window.PollEvents();

        uint32_t imageIndex;
        if (!device.BeginFrame(imageIndex))
        {
            continue;
        }

        commandBuffer.Reset();
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
    }

    device.WaitIdle();
    delete pipeline;
    
    return 0;
}