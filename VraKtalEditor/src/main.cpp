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
    #pragma comment(lib, "VraKtalEngine_Debug.lib")
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

    auto* matLayout = renderer.GetPass<graphics::GBufferPass>("GBuffer")->GetMaterialLayout();

    loader.LoadMesh("assets/models/cave.obj",
        [&](std::shared_ptr<graphics::resources::Mesh> mesh)
        {
            graphics::resources::object::Material mat0;
            mat0.SetTexture("assets/textures/extracted_textures/diffuse_sand.jpg.png", "albedo");
            mat0.SetTexture("assets/textures/extracted_textures/normal_sand.png.png", "normal");
            mat0.SetMetallicRoughness(0.0, 0.75f);
           
            
            graphics::resources::object::Material mat1;
            mat1.SetTexture("assets/textures/extracted_textures/diffus_rocktill_02.jpg.png", "albedo");
            mat1.SetTexture("assets/textures/extracted_textures/normal_rocktill_02.png.png", "normal");
            mat1.SetMetallicRoughness(0.0, 0.85f);
            
            graphics::resources::object::Material mat2;
            mat2.SetTexture("assets/textures/extracted_textures/diffus_plane_watreplant.tga.png", "albedo");
            mat2.SetTexture("assets/textures/extracted_textures/normal_plane_watreplant.png.png", "normal");
            mat2.SetMetallicRoughness(0.0, 0.65f);

            graphics::resources::object::Material mat3;
            mat3.SetTexture("assets/textures/extracted_textures/diffus_foliage_03_copy.tga.png", "albedo");
            mat3.SetTexture("assets/textures/extracted_textures/normal_foliage_03.png.png", "normal");
            mat3.SetMetallicRoughness(0.0, 0.78f);
            
            graphics::resources::object::Material mat4;
            mat4.SetTexture("assets/textures/extracted_textures/diffus_foliage_03_copy.tga.png", "albedo");
            mat4.SetTexture("assets/textures/extracted_textures/normal_foliage_03.png.png", "normal");
            mat4.SetMetallicRoughness(0.0, 0.78f);
           
            graphics::resources::object::Material mat5;
            mat5.SetTexture("assets/textures/extracted_textures/diffuse_cave_foliage.tga.png", "albedo");
            mat5.SetTexture("assets/textures/extracted_textures/normal_cave_foliage.png.png", "normal");
            mat5.SetMetallicRoughness(0.0, 0.60f);
            
            graphics::resources::object::Material mat6; 
            mat6.SetTexture("assets/textures/extracted_textures/diffuse_cave.jpg.png", "albedo");
            mat6.SetTexture("assets/textures/extracted_textures/normal_cave.png.png", "normal");
            mat6.SetMetallicRoughness(0.0, 0.72f);

            graphics::resources::object::Material mat7;
            mat7.SetTexture("assets/textures/extracted_textures/diffuse_grass.jpg.png", "albedo");
            mat7.SetTexture("assets/textures/extracted_textures/normal_grass.png.png", "normal");
            mat7.SetMetallicRoughness(0.0, 0.88f);
            
            graphics::resources::object::Material mat8;
            mat8.SetTexture("assets/textures/extracted_textures/diffuse_cave_indoor.jpg.png", "albedo");
            mat8.SetTexture("assets/textures/extracted_textures/normal_cave_indoor.png.png", "normal");
            mat8.SetMetallicRoughness(0.0, 0.82f);
            
            graphics::resources::object::Material mat9;
            mat9.SetTexture("assets/textures/extracted_textures/diffuse_cave_bole.jpg.png", "albedo");
            mat9.SetTexture("assets/textures/extracted_textures/normal_cave_bole.png.png", "normal");
            mat9.SetMetallicRoughness(0.0, 0.90f);
            
            graphics::resources::object::Material mat10;
            mat10.SetTexture("assets/textures/extracted_textures/diffuse_cave_foliage.tga.png", "albedo");
            mat10.SetTexture("assets/textures/extracted_textures/normal_cave_foliage.png.png", "normal");
            mat10.SetMetallicRoughness(0.0, 0.60f);
            
            graphics::resources::object::Material mat11;
            mat11.SetTexture("assets/textures/extracted_textures/diffus_rocktill_02.jpg.png", "albedo");
            mat11.SetTexture("assets/textures/extracted_textures/normal_rocktill_02.png.png", "normal");
            mat11.SetMetallicRoughness(0.0, 0.80f);
            
            graphics::resources::object::Material mat12;
            mat12.SetTexture("assets/textures/extracted_textures/diffus_rocks.jpg.png", "albedo");
            mat12.SetTexture("assets/textures/extracted_textures/normal_rocks.png.png", "normal");
            mat12.SetMetallicRoughness(0.0, 0.78f);
            
            graphics::resources::object::Material mat13;
            mat13.SetTexture("assets/textures/extracted_textures/diffuse_cave_stones.jpg.png", "albedo");
            mat13.SetTexture("assets/textures/extracted_textures/normal_cave_stones.png.png", "normal");
            mat13.SetMetallicRoughness(0.0, 0.70f);
            
            graphics::resources::object::Material mat14;
            mat14.SetTexture("assets/textures/extracted_textures/diffuse_skeleton.jpg.png", "albedo");
            mat14.SetTexture("assets/textures/extracted_textures/normal_skeleton.png.png", "normal");
            mat14.SetMetallicRoughness(0.0, 0.85f);
            
            graphics::resources::object::Material mat15;
            mat15.SetTexture("assets/textures/extracted_textures/diffus_fetich.jpg.png", "albedo");
            mat15.SetTexture("assets/textures/extracted_textures/normal_fetich.png.png", "normal");
            mat15.SetMetallicRoughness(0.0, 0.68f);
            
            graphics::resources::object::Material mat16;
            mat16.SetTexture("assets/textures/extracted_textures/diffus_grass-transition_clairiere.jpg.png", "albedo");
            mat16.SetMetallicRoughness(0.0, 0.86f);

            mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat0, matLayout));
            mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat1, matLayout));
            mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat2, matLayout));
            mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat3, matLayout));
            mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat4, matLayout));            
            mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat5, matLayout));            
            mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat6, matLayout));
            mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat7, matLayout));
            mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat8, matLayout));
            mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat9, matLayout));
            mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat10, matLayout));
            mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat11, matLayout));
            mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat12, matLayout));
            mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat13, matLayout));
            mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat14, matLayout));
            mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat15, matLayout));
            mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat16, matLayout));

            vikingRoomMesh = mesh;
        });

    loader.ProcessJobs();
    loader.PurgeFinishedJobs();

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

        glm::mat4 meshTransform1 = glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, 2.0f, 0.0f));
        meshTransform1 = glm::rotate(meshTransform1, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        renderer.PushMesh(vikingRoomMesh.get(), meshTransform1);

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
        light3.position = glm::vec3(0.0f, 2.0f, 0.0f);
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