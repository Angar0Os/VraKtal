#include <iostream>

#include <core/window.h>
#include <core/gpu/device.h>
#include <graphics/renderer.h>

#include <loaders/meshLoader.h>
#include <graphics/resources/scene.h>

#pragma comment(lib, "VraKtalEngine_Debug.lib")

int main()
{
    try
    {
        core::Window window(800, 600, "VraKtal");
        core::gpu::Device device(window);

        graphics::Renderer renderer(window, device);
        
        loaders::MeshLoader meshLoader;
        auto mesh = meshLoader.LoadMesh("../bin/assets/models/viking_room.obj");

        graphics::resources::Scene scene;
        scene.AddMesh(mesh);

        while (!window.ShouldClose())
        {
            renderer.DrawFrame(/*scene*/);
            window.PollEvents();
        }

        renderer.Cleanup();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}