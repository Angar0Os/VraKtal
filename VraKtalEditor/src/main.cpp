#include <iostream>

#include <core/window.h>
#include <core/gpu/device.h>
#include <graphics/renderer.h>

#include <loaders/meshLoader.h>
#include <graphics/resources/scene.h>

#include <glm/gtc/matrix_transform.hpp>

#pragma comment(lib, "VraKtalEngine_Debug.lib")

int main()
{
    core::Window window(800, 600, "VraKtal Engine");
    core::gpu::Device device(window);
    graphics::Renderer renderer(window, device);

    loaders::MeshLoader loader;
    auto mesh = loader.LoadMesh("assets/models/viking_room.obj");

    std::cout << "Mesh loaded: "
        << mesh->vertices.size() << " vertices, "
        << mesh->indices.size() << " indices" << std::endl;

    if (mesh->vertices.empty() || mesh->indices.empty())
    {
        std::cerr << "ERROR: Mesh is empty!" << std::endl;
        return -1;
    }
        
    auto scene = std::make_shared<graphics::resources::Scene>();
    scene->AddMesh(mesh, glm::mat4(1.0f));
    renderer.SetScene(scene);

    glm::vec3 cameraPos(2.0f, 2.0f, 2.0f);
    glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 10.0f);
    proj[1][1] *= -1;

    renderer.UpdateCamera(view, proj, cameraPos);

    while (!window.ShouldClose())
    {
        window.PollEvents();
        renderer.DrawFrame();
    }

    renderer.Cleanup();
    return 0;
}