
#include <core/window.h>
#include <core/gpu/device.h>

#pragma comment(lib, "VraKtalEngine_Debug.lib")

int main()
{
    core::Window window(800, 600, "Texture Mesh");
    core::gpu::Device device(window);
 /*   core::gpu::Renderer(device) renderer;
    

    device.InitVulkan();
    renderer.MainLoop();
    renderer.Cleanup();*/

    return 0;
}