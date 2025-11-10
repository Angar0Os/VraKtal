#include <iostream>

#include <core/window.h>
#include <core/gpu/device.h>
#include <graphics/renderer.h>

#include <chrono>
#include <thread>

#pragma comment(lib, "VraKtalEngine_Debug.lib")

int main()
{
    try
    {
        core::Window window(800, 600, "Texture Mesh");
        core::gpu::Device device(window);

        graphics::Renderer renderer(window, device);

        renderer.SetFrameCallback([&](uint32_t imageIndex, void* available, void* finished, void* inFlightFence) -> bool {
            std::cout << "Frame for image " << imageIndex << "\n";
            (void)available; (void)finished; (void)inFlightFence;
            return false;
            });

        while (!window.ShouldClose())
        {
            renderer.DrawFrame();
            window.PollEvents();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
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