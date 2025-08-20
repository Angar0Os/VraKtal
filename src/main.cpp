#include "core/gpu/gpuDevice_impl_glfw_vulkan.h"

int main(int argc, char** argv)
{
	rhi::core::gpu::GpuDevice gpuDevice{
		{
			.windowTitle = "VraKtal Engine",
			.windowSize = { 1280, 720 },
			.resizable = true
		}
	};

	// do
	// {
	//
	// } while (gpuDevice.Present());

	return 0;
}