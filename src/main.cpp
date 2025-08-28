#include "core/gpu/window_impl_vulkan.h"
#include "core/gpu/gpuDevice_impl_glfw_vulkan.h"
#include "core/gpu/commandBuffer_impl_vulkan.h"

#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "vulkan-1.lib")

int main(int argc, char** argv)
{
	rhi::vulkan::WindowVulkan window(1280, 720, "Vulkan Window");
	rhi::vulkan::GpuDeviceVulkan device(window);

	rhi::core::gpu::CommandBuffer* commandBuffer = device.CreateCommandBuffer();
	commandBuffer->Begin();
	commandBuffer->End();

	while (!window.ShouldClose())
	{
		window.PollEvents();
	}

	device.DestroyCommandBuffer(commandBuffer);
	device.WaitIdle();
}