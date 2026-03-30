#include <iostream>

#include <core/window.h>
#include <core/gpu/device.h>
#include <core/gpu/image.h>
#include <core/gpu/imguiContext.h>

#include <graphics/renderer.h>
#include <graphics/renderPass/gBufferPass.h>
#include <graphics/resources/object/light.h>
#include <graphics/resources/object/material.h>

#include <loaders/meshLoader.h>
#include <loaders/materialLoader.h>

#include <demo/scene.h>

#include "imGuiWindows.h"
#include "utils/yamlParser.h"

#pragma comment(lib, "VraKtalEngine_Debug.lib")

#define VRAKTAL_EDITOR

int main()
{
    core::Window        window(800, 600, "VraKtal Engine");
    core::gpu::Device   device(window);
    graphics::Renderer  renderer(window, device);
    loaders::MeshLoader loader(&device);

    auto vikingRoomMesh = loader.LoadMesh("assets/models/viking_room.obj");
    auto planeMesh = loader.CreatePlane(10.0f, 10.0f, 10, 10);

    auto* matLayout = renderer.GetPass<graphics::GBufferPass>("GBuffer")->GetMaterialLayout();

    {
        graphics::resources::object::Material mat;
        mat.SetTexture("assets/textures/viking_room.png", "albedo");
        mat.SetMetallicRoughness(0.0f, 0.8f);
        vikingRoomMesh->materials.push_back(
            loaders::MaterialLoader::Load(device, mat, matLayout));
    }
    {
        graphics::resources::object::Material mat;
        mat.SetAlbedo(0.9f, 0.0f, 0.2f);
        mat.SetMetallicRoughness(0.0f, 0.9f);
        planeMesh->materials.push_back(
            loaders::MaterialLoader::Load(device, mat, matLayout));
    }

    auto mainCamera = std::make_unique<graphics::resources::object::Camera>("mainCamera");
    mainCamera->fov = 45.0f;
    mainCamera->aspectRatio = 800.0f / 600.0f;
    mainCamera->transform.SetPosition(glm::vec3(0.0f, 3.0f, -5.0f));
    mainCamera->LookAt(glm::vec3(0.0f, 0.0f, 0.0f));

    graphics::resources::property::Transform planeTransform;

    graphics::resources::property::Transform vikingTransform1;
    vikingTransform1.SetPosition(glm::vec3(-1.5f, 0.1f, 0.0f));
    vikingTransform1.Rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    graphics::resources::property::Transform vikingTransform2;
    vikingTransform2.SetPosition(glm::vec3(1.5f, 0.1f, 0.0f));
    vikingTransform2.Rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    auto light1 = graphics::resources::Light::CreatePointLight(
        glm::vec3(0.0f), glm::vec3(1.0f, 0.9f, 0.2f), 10.0f, "Yellow Light 1");
    light1.radius = 0.1f;

    auto light2 = graphics::resources::Light::CreatePointLight(
        glm::vec3(0.0f), glm::vec3(1.0f, 0.85f, 0.1f), 8.0f, "Yellow Light 2");
    light2.radius = 0.1f;

    auto light3 = graphics::resources::Light::CreatePointLight(
        glm::vec3(0.0f, 6.0f, 0.0f), glm::vec3(0.2f, 0.4f, 1.0f), 15.0f, "Blue Light");
    light3.radius = 0.1f;

    demo::Scene scene("mainScene");
    scene.Add({ "mainCamera",  mainCamera.get(),     {},               false });
    scene.Add({ "plane",       planeMesh.get(),       planeTransform,   false });
    scene.Add({ "vikingRoom1", vikingRoomMesh.get(),  vikingTransform1, true });
    scene.Add({ "vikingRoom2", vikingRoomMesh.get(),  vikingTransform2, true });
    scene.Add({ "light1",      &light1,               {},               false });
    scene.Add({ "light2",      &light2,               {},               false });
    scene.Add({ "light3",      &light3,               {},               false });

    ImGuiWindows imGuiWindows(
        device.GetImGuiContext(),
        &renderer,
        &window,
        &scene,
        &loader
    );

    device.GetImGuiContext()->BindPrepareDrawData([&]()
        {
            imGuiWindows.PrepareImGuiWindows();
        });

    float    time = 0.0f;
    const float timeStep = (1.0f / 240.0f) / 5.0f;
    uint32_t currentFrameIndex = 0;

    while (!window.ShouldClose())
    {
        window.PollEvents();

        if (device.NeedsResize())
        {
            device.RecreateSwapchain();
            renderer.OnResize();
            device.GetImGuiContext()->OnResize();
            device.ClearResizeFlag();
            continue;
        }

        uint32_t imageIndex = device.AcquireNextImage(currentFrameIndex);
        if (imageIndex == UINT32_MAX)
        {
            device.RecreateSwapchain();
            renderer.OnResize();
            device.GetImGuiContext()->OnResize();
            continue;
        }

        time += timeStep;

        light1.position = glm::vec3(3.0f * glm::cos(time), 4.0f, 3.0f * glm::sin(time));
        light2.position = glm::vec3(
            3.0f * glm::cos(time + glm::pi<float>()),
            3.0f,
            3.0f * glm::sin(time + glm::pi<float>()));

        if (auto* cam = scene.GetActiveCamera())
            renderer.SetCamera(cam->GetViewMatrix(), cam->GetProjectionMatrix());

        for (const auto& resource : scene.sceneObjects)
        {
            std::visit([&](auto* obj)
                {
                    using T = std::decay_t<decltype(*obj)>;
                    if constexpr (std::is_same_v<T, graphics::resources::Mesh>)
                    {
                        renderer.PushMesh(obj, resource.objectTransform.GetMatrix());
                    }
                    else if constexpr (std::is_same_v<T, graphics::resources::Light>)
                    {
                        if (obj && obj->enabled)
                            renderer.PushLight(*obj);
                    }
                }, resource.object);
        }

#ifndef VRAKTAL_EDITOR
        renderer.Render(device.GetSwapchainImage(imageIndex), ImageLayout::Present);
#else
        imGuiWindows.GetContext()->PrepareForDrawing();
        renderer.Render(imGuiWindows.GetContext()->GetViewportImage(), ImageLayout::ShaderReadOnly);

        auto cmd = renderer.GetCurrentCommandBuffer();
        auto swapchainImage = device.GetSwapchainImage(imageIndex);

        CommandBuffer::RenderingAttachmentInfo imguiColor{};
        imguiColor.image = swapchainImage;
        imguiColor.clear = false;

        CommandBuffer::DepthAttachmentInfo noDepth{};
        noDepth.image = nullptr;

        cmd->TransitionImageLayout(swapchainImage, ImageLayout::Undefined, ImageLayout::ColorAttachment, false);
        cmd->BeginRendering(&device, { imguiColor }, noDepth);
        imGuiWindows.GetContext()->PrepareDrawData();
        imGuiWindows.GetContext()->DrawEditors(cmd);
        cmd->EndRendering();
        cmd->TransitionImageLayout(swapchainImage, ImageLayout::ColorAttachment, ImageLayout::Present, false);
#endif

        renderer.Advance();
        device.Present(imageIndex, currentFrameIndex);
        currentFrameIndex = (currentFrameIndex + 1) % core::gpu::Device::s_FRAMES_IN_FLIGHT;
    }

    device.WaitIdle();
    renderer.Cleanup();

    return 0;
}