#include <iostream>
#include <core/window.h>
#include <core/gpu/device.h>
#include <core/gpu/image.h>
#include <graphics/renderer.h>
#include <graphics/renderPass/gBufferPass.h>
#include <graphics/resources/object/light.h>
#include <graphics/resources/object/material.h>
#include <loaders/meshLoader.h>
#include <loaders/materialLoader.h>
#include "imGuiWindows.h"
#include "utils/yamlParser.h"
#include <core/input/input.h>

#ifdef VRAKTAL_EDITOR
    #ifdef _DEBUG
        #pragma comment(lib, "VraKtalEngine_Debug.lib") //_Debug
    #else
        #pragma comment(lib, "VraKtalEngine.lib")
    #endif
    #include <core/gpu/imguiContext.h>
#else
    #pragma comment(lib, "VraKtalEngine.lib")
#endif // VRAKTAL_EDITOR


class App
{
public:
    App(core::Input& _input) {
        _input.AddAction("CloseApp");
        _input.BindActionKey({ input::Key::ESCAPE }, "CloseApp");
        _input.BindActionCallback<App, &App::CloseApp>("CloseApp", this, input::KeyState::Press);
    };
    ~App() {};

    bool ShouldClose() const { return bSouldCloseApp; }
    void CloseApp() {
        bSouldCloseApp = true;
        std::cout << "Close App Action Triggered" << std::endl;
    };

private:
    bool bSouldCloseApp = false;
};

class Camera
{
public:
    float aspectRatio;
    glm::mat4 projection;
    glm::vec3 cameraPosition;
    glm::vec3 direction;

    Camera(core::Input& _input) : direction(0.0f, 0.0f, -1.0f)
    {
        aspectRatio = 800.0f / 600.0f;
        projection = glm::perspectiveLH_ZO(
            glm::radians(45.0f),
            aspectRatio,
            0.1f,
            100.0f
        );
        projection[1][1] *= -1;

        cameraPosition = glm::vec3(0.0f, 0.0f, 5.0f);

        _input.BindMouseCallback<Camera, &Camera::Look>(this);
        
        _input.AddAction("CameraLook");
        _input.BindActionKey({ input::Key::GLFW_MOUSE_BUTTON_RIGHT}, "CameraLook");
        _input.BindActionCallback<Camera, &Camera::EnableLook>("CameraLook", this, input::KeyState::Press);
        _input.BindActionCallback<Camera, &Camera::DisableLook>("CameraLook", this, input::KeyState::Release);

        _input.AddAxis2DAction("MoveCamera", input::Key::D, input::Key::A, input::Key::W, input::Key::S);
        _input.BindAxis2DCallack<Camera, &Camera::MoveCamera>("MoveCamera", this);

        _input.AddAction("MoveCameraUp");
        _input.BindActionKey({ input::Key::SPACE }, "MoveCameraUp");
        _input.BindActionCallback<Camera, &Camera::MoveCameraUp>("MoveCameraUp", this, input::KeyState::OnGoing);
    };
    ~Camera() {};

    void MoveCamera(glm::vec2 value) {
        float speed = 0.05;
        cameraPosition += (value.y * speed) * direction;

        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 cameraRight = glm::normalize(glm::cross(up, direction));

        cameraPosition += (value.x * speed) * cameraRight;
    }

    void MoveCameraUp() {
        float speed = 0.05;
        cameraPosition.y += speed;
    }

    void Look(glm::vec2 mouseDelta) {
        if (!bReceiveInputs)
        {
            return;
        }
        float sensitivity = 0.1f;
        static float yaw = -90.0f;
        static float pitch = 0.0f;
        yaw -= mouseDelta.x * sensitivity;
        pitch += mouseDelta.y * sensitivity;

        // Clamp the pitch to prevent flipping
        pitch = glm::clamp(pitch, -89.0f, 89.0f);

        // Calculate the new camera direction
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction = glm::normalize(direction);
    }
    bool bReceiveInputs = false;

    void EnableLook() { 
        bReceiveInputs = true;
    }
    void DisableLook(){ 
        bReceiveInputs = false; 
    }

    glm::mat4 GetView() const { return glm::lookAtLH(cameraPosition, cameraPosition + direction, glm::vec3(0.0f, 1.0f, 0.0f)); }
};

int main()
{
    core::Window window(800, 600, "VraKtal Engine");
    core::gpu::Device device(window);
    core::Input input(window, &device);
    graphics::Renderer renderer(window, device);
#ifdef VRAKTAL_EDITOR
    ImGuiWindows imGuiWindows = ImGuiWindows(device.GetImGuiContext(), &renderer, &window, input);
    device.GetImGuiContext()->BindPrepareDrawData([&]()
        {
            imGuiWindows.PrepareImGuiWindows();
        });
#endif //VRAKTAL_EDITOR



    loaders::MeshLoader loader(&device);

    App app(input);
    Camera camera(input);

    std::shared_ptr<graphics::resources::Mesh> vikingRoomMesh;
    std::shared_ptr<graphics::resources::Mesh> planeMesh;

    loader.LoadMesh("assets/models/viking_room.obj",
        [&](std::shared_ptr<graphics::resources::Mesh> mesh)
        {
            vikingRoomMesh = mesh;
        });

    loader.CreatePlane(10.0f, 10.0f, 10, 10,
        [&](std::shared_ptr<graphics::resources::Mesh> mesh)
        {
            planeMesh = mesh;
        });

    loader.ProcessJobs();
    loader.PurgeFinishedJobs();

    auto* matLayout = renderer.GetPass<graphics::GBufferPass>("GBuffer")->GetMaterialLayout();

    {
        graphics::resources::object::Material mat;
        mat.SetTexture("assets/textures/viking_room.png", "albedo");
        mat.SetMetallicRoughness(0.0f, 0.8f);
        vikingRoomMesh->materials.push_back(
            loaders::MaterialLoader::Load(device, mat, matLayout)
        );
    }

    {
        graphics::resources::object::Material mat;
        mat.SetAlbedo(0.9f, 0.0f, 0.2f);
        mat.SetMetallicRoughness(0.0f, 0.9f);
        planeMesh->materials.push_back(
            loaders::MaterialLoader::Load(device, mat, matLayout)
        );
    }

    const float aspectRatio = 800.0f / 600.0f;
    glm::mat4 projection = glm::perspectiveLH_ZO(
        glm::radians(45.0f),
        aspectRatio,
        0.1f,
        100.0f
    );
    projection[1][1] *= -1;

    glm::vec3 cameraPosition = glm::vec3(0.0f, 3.0f, -5.0f);
    glm::mat4 view = glm::lookAtLH(
        cameraPosition,
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    float       time = 0.0f;
    const float timeStep = (1.0f / 240.0f) / 5.0f;

    uint32_t currentFrameIndex = 0;
    uint32_t frameCounter = 0;

    utils::YamlParser parser("project.yaml");
    std::vector<graphics::resources::Light> lights;
    if (parser.IsValid())
        lights = parser.LoadLights();

    while (!window.ShouldClose() && !app.ShouldClose())
    {
        window.PollEvents();
        input.Update();

        if (device.NeedsResize())
        {
            device.RecreateSwapchain();
            renderer.OnResize();
            device.ClearResizeFlag();
#ifdef VRAKTAL_EDITOR
            device.GetImGuiContext()->OnResize();
#endif // VRAKTAL_EDITOR

            continue;
        }

        uint32_t imageIndex = device.AcquireNextImage(currentFrameIndex);
        if (imageIndex == UINT32_MAX)
        {
            device.RecreateSwapchain();
            renderer.OnResize();
#ifdef VRAKTAL_EDITOR
            device.GetImGuiContext()->OnResize();
#endif // VRAKTAL_EDITOR
            continue;
        }

        time += timeStep;

        renderer.SetCamera(camera.GetView(), camera.projection);

        renderer.PushMesh(planeMesh.get(), glm::mat4(1.0f));

        glm::mat4 meshTransform1 = glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, 0.1f, 0.0f));
        meshTransform1 = glm::rotate(meshTransform1, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        renderer.PushMesh(vikingRoomMesh.get(), meshTransform1);

        glm::mat4 meshTransform2 = glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 0.1f, 0.0f));
        meshTransform2 = glm::rotate(meshTransform2, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        renderer.PushMesh(vikingRoomMesh.get(), meshTransform2);

        graphics::resources::Light light1;
        light1.name = "Yellow Light 1";
        light1.position = glm::vec3(3.0f * glm::cos(time), 4.0f, 3.0f * glm::sin(time));
        light1.color = glm::vec3(1.0f, 0.9f, 0.2f);
        light1.intensity = 10.0f;
        light1.radius = 0.1f;
        light1.enabled = true;
        renderer.PushLight(light1);

        graphics::resources::Light light2;
        light2.name = "Yellow Light 2";
        light2.position = glm::vec3(3.0f * glm::cos(time + glm::pi<float>()), 3.0f, 3.0f * glm::sin(time + glm::pi<float>()));
        light2.color = glm::vec3(1.0f, 0.85f, 0.1f);
        light2.intensity = 8.0f;
        light2.radius = 0.1f;
        light2.enabled = true;
        renderer.PushLight(light2);

        graphics::resources::Light light3;
        light3.name = "Blue Light";
        light3.position = glm::vec3(0.0f, 6.0f, 0.0f);
        light3.color = glm::vec3(0.2f, 0.4f, 1.0f);
        light3.intensity = 15.0f;
        light3.radius = 0.1f;
        light3.enabled = true;
        renderer.PushLight(light3);

#ifndef VRAKTAL_EDITOR
        renderer.Render(device.GetSwapchainImage(imageIndex), ImageLayout::Present);
#else
        imGuiWindows.GetContext()->PrepareForDrawing();
        auto image = imGuiWindows.GetContext()->GetViewportImage();

        renderer.Render(imGuiWindows.GetContext()->GetViewportImage(), ImageLayout::ShaderReadOnly);
        auto cmd = renderer.GetCurrentCommandBuffer();
        auto swapchainImage = device.GetSwapchainImage(imageIndex);

        CommandBuffer::RenderingAttachmentInfo imguiColor{};
        imguiColor.image = swapchainImage;
        imguiColor.clear = false;

        CommandBuffer::DepthAttachmentInfo noDepth{};
        noDepth.image = nullptr;

        cmd->TransitionImageLayout(
            swapchainImage,
            ImageLayout::Undefined,
            ImageLayout::ColorAttachment,
            false
        );

        cmd->BeginRendering(&device, { imguiColor }, noDepth);

        imGuiWindows.GetContext()->PrepareDrawData();
        imGuiWindows.GetContext()->DrawEditors(cmd);
        cmd->EndRendering();

        cmd->TransitionImageLayout(
            swapchainImage,
            ImageLayout::ColorAttachment,
            ImageLayout::Present,
            false
        );
#endif
        renderer.Advance();
        device.Present(imageIndex, currentFrameIndex);
        currentFrameIndex = (currentFrameIndex + 1) % core::gpu::Device::s_FRAMES_IN_FLIGHT;
        frameCounter++;
    }

    device.WaitIdle();
    renderer.Cleanup();

    return 0;
}