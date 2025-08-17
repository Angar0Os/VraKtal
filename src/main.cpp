#include "core/renderContext_impl_glfw_vulkan.h"

#include "editor/mainEditor.h"

int main(int argc, char** argv)
{
	bool initEditor = true;

	core::rhi::RenderContext rCtx{
{
		.windowTitle = "Vraktal Engine",
		.windowSize = { 1280, 720 },
		.resizeable = true
		}
	};

	editor::MainEditor mainEditor(rCtx);

	do
	{
		if (initEditor)
			mainEditor.RunImgui();
	} while (rCtx.Present());

	mainEditor.ClearImgui();

	return 0;
}