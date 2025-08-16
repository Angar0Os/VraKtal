#include "core/renderContext_impl_glfw_vulkan.h"

int main(int argc, char** argv)
{
	core::rhi::RenderContext rCtx{
		{
			.windowTitle = "Vraktal Engine",
			.windowSize = { 1280, 720 },
			.resizeable = true
		}
	};

	rCtx.GetInternal().Run();
}