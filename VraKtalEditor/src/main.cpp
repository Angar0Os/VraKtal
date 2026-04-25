#include <iostream>
#include <core/window.h>
#include <core/gpu/device.h>
#include <core/gpu/image.h>
#include <graphics/renderer.h>
#include <graphics/renderPass/gBufferPass.h>
#include <graphics/resources/object/light.h>
#include <graphics/resources/object/material.h>
#include <core/manager/ressourceManager.h>

#include <scene/system/systemManager.h>
#include <scene/system/systems/meshSystem.h>

#include <loaders/materialLoader.h>
#include "imGuiWindows.h"
#include "utils/yamlParser.h"
#include <core/input/input.h>

#include <core/manager/ressourceManager.h>

#include <scene/system/systemManager.h>
#include <scene/system/systems/meshSystem.h>

#include <scene/scene.h>
#include <scene/timeline/components/mesh.h>
#include <scene/timeline/components/light.h>

#ifdef VRAKTAL_EDITOR
    #pragma comment(lib, "VraKtalEngine_Debug.lib")
    #include <core/gpu/imguiContext.h>
#else
    #pragma comment(lib, "VraKtalEngine.lib")
#endif // VRAKTAL_EDITOR
#include <scene/system/systems/lightSystem.h>


class App
{
public:
    App(core::Input& _input , core::gpu::Device& _device , graphics::Renderer& _renderer) {
        _input.AddAction("CloseApp");
        _input.BindActionKey({ input::Key::ESCAPE }, "CloseApp");
        _input.BindActionCallback<App, &App::CloseApp>("CloseApp", this, input::KeyState::Press);

        _input.AddAction("SpawnVikingRoom");
        _input.BindActionKey({ input::Key::F }, "SpawnVikingRoom");
        _input.BindActionCallback<App, &App::SpawnVikingRoom>("SpawnVikingRoom", this, input::KeyState::Press);

        m_scene = new Scene();
        m_scene->RegisterComponentStorage<timeline::MeshInstance>();
        m_scene->RegisterComponentStorage<timeline::Light>();

        m_reManager = new RessourceManager(&_device);
        m_reManager->LoadRessource<graphics::resources::Mesh>("assets/models/viking_room.obj");
        auto* matLayout = _renderer.GetPass<graphics::GBufferPass>("GBuffer")->GetMaterialLayout();
        {
            graphics::resources::object::Material mat;
            mat.SetTexture("assets/textures/viking_room.png", "albedo");
            mat.SetMetallicRoughness(0.0f, 0.8f);
            m_reManager->GetRessource<graphics::resources::Mesh>("assets/models/viking_room.obj").materials.push_back(
                loaders::MaterialLoader::Load(_device, mat, matLayout)
            );
        }
        timeline::MeshInstance timelineMesh;
        timelineMesh.meshID = m_reManager->GetRessourceID<graphics::resources::Mesh>("assets/models/viking_room.obj");
        m_scene->CreateEntity<timeline::MeshInstance>(timelineMesh);
    };
    ~App() {
        delete m_scene;
        delete m_reManager;
    };

    bool ShouldClose() const { return bSouldCloseApp; }
    void CloseApp() {
        bSouldCloseApp = true;
        std::cout << "Close App Action Triggered" << std::endl;
    };

    void SpawnVikingRoom() {
        std::cout << "Spawn Viking Room Action Triggered" << std::endl;
        timeline::MeshInstance timelineMesh;
        timelineMesh.meshID = m_reManager->GetRessourceID<graphics::resources::Mesh>("assets/models/viking_room.obj");
        m_scene->CreateEntity<timeline::MeshInstance>(timelineMesh);

    }

    Scene* m_scene;
    RessourceManager* m_reManager;
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

    App app(input , device , renderer);
    Camera camera(input);

    SystemManager systemManager;
    systemManager.AddSystem<MeshSystem>(&renderer, app.m_reManager);
    systemManager.AddSystem<LightSystem>(&renderer);

#ifdef VRAKTAL_EDITOR
    ImGuiWindows imGuiWindows = ImGuiWindows(device.GetImGuiContext(), &renderer, &window, input , app.m_scene ,*app.m_reManager);
    device.GetImGuiContext()->BindPrepareDrawData([&]()
        {
            imGuiWindows.DrawImGui();
        });
#endif //VRAKTAL_EDITOR
    std::shared_ptr<graphics::resources::Mesh> vikingRoomMesh;

    auto* matLayout = renderer.GetPass<graphics::GBufferPass>("GBuffer")->GetMaterialLayout();

    //loader.LoadMesh("assets/models/cave.obj",
    //    [&](std::shared_ptr<graphics::resources::Mesh> mesh)
    //    {
    //        graphics::resources::object::Material mat0;
    //        mat0.SetTexture("assets/textures/extracted_textures/diffuse_sand.jpg.png", "albedo");
    //        mat0.SetTexture("assets/textures/extracted_textures/normal_sand.png.png", "normal");
    //        mat0.SetMetallicRoughness(0.0, 0.75);
    //        mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat0, matLayout));
    //        
    //        graphics::resources::object::Material mat1;
    //        mat1.SetTexture("assets/textures/extracted_textures/diffus_rocktill_02.jpg.png", "albedo");
    //        mat1.SetTexture("assets/textures/extracted_textures/normal_rocktill_02.png.png", "normal");
    //        mat1.SetMetallicRoughness(0.0, 0.85);
    //        mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat1, matLayout));
    //        
    //        graphics::resources::object::Material mat2;
    //        mat2.SetTexture("assets/textures/extracted_textures/diffus_plane_watreplant.tga.png", "albedo");
    //        mat2.SetTexture("assets/textures/extracted_textures/normal_plane_watreplant.png.png", "normal");
    //        mat2.SetMetallicRoughness(0.0, 0.65);
    //        mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat2, matLayout));

    //        graphics::resources::object::Material mat3;
    //        mat3.SetTexture("assets/textures/extracted_textures/diffus_foliage_03_copy.tga.png", "albedo");
    //        mat3.SetTexture("assets/textures/extracted_textures/normal_foliage_03.png.png", "normal");
    //        mat3.SetMetallicRoughness(0.0, 0.78);
    //        mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat3, matLayout));
    //        
    //        graphics::resources::object::Material mat4;
    //        mat4.SetTexture("assets/textures/extracted_textures/diffus_foliage_03_copy.tga.png", "albedo");
    //        mat4.SetTexture("assets/textures/extracted_textures/normal_foliage_03.png.png", "normal");
    //        mat4.SetMetallicRoughness(0.0, 0.78);
    //        mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat4, matLayout));            
    //       
    //        graphics::resources::object::Material mat5;
    //        mat5.SetTexture("assets/textures/extracted_textures/diffuse_cave_foliage.tga.png", "albedo");
    //        mat5.SetTexture("assets/textures/extracted_textures/normal_cave_foliage.png.png", "normal");
    //        mat5.SetMetallicRoughness(0.0, 0.60);
    //        mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat5, matLayout));            
    //        
    //        graphics::resources::object::Material mat6; 
    //        mat6.SetTexture("assets/textures/extracted_textures/diffuse_cave.jpg.png", "albedo");
    //        mat6.SetTexture("assets/textures/extracted_textures/normal_cave.png.png", "normal");
    //        mat6.SetMetallicRoughness(0.0, 0.72);
    //        mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat6, matLayout));

    //        graphics::resources::object::Material mat7;
    //        mat7.SetTexture("assets/textures/extracted_textures/diffuse_grass.jpg.png", "albedo");
    //        mat7.SetTexture("assets/textures/extracted_textures/normal_grass.png.png", "normal");
    //        mat7.SetMetallicRoughness(0.0, 0.88);
    //        mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat7, matLayout));
    //        
    //        graphics::resources::object::Material mat8;
    //        mat8.SetTexture("assets/textures/extracted_textures/diffuse_cave_indoor.jpg.png", "albedo");
    //        mat8.SetTexture("assets/textures/extracted_textures/normal_cave_indoor.png.png", "normal");
    //        mat8.SetMetallicRoughness(0.0, 0.82);
    //        mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat8, matLayout));
    //        
    //        graphics::resources::object::Material mat9;
    //        mat9.SetTexture("assets/textures/extracted_textures/diffuse_cave_bole.jpg.png", "albedo");
    //        mat9.SetTexture("assets/textures/extracted_textures/normal_cave_bole.png.png", "normal");
    //        mat9.SetMetallicRoughness(0.0, 0.90);
    //        mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat9, matLayout));
    //        
    //        graphics::resources::object::Material mat10;
    //        mat10.SetTexture("assets/textures/extracted_textures/diffuse_cave_foliage.tga.png", "albedo");
    //        mat10.SetTexture("assets/textures/extracted_textures/normal_cave_foliage.png.png", "normal");
    //        mat10.SetMetallicRoughness(0.0, 0.60);
    //        mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat10, matLayout));
    //        
    //        graphics::resources::object::Material mat11;
    //        mat11.SetTexture("assets/textures/extracted_textures/diffus_rocktill_02.jpg.png", "albedo");
    //        mat11.SetTexture("assets/textures/extracted_textures/normal_rocktill_02.png.png", "normal");
    //        mat11.SetMetallicRoughness(0.0, 0.80);
    //        mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat11, matLayout));
    //        
    //        graphics::resources::object::Material mat12;
    //        mat12.SetTexture("assets/textures/extracted_textures/diffus_rocks.jpg.png", "albedo");
    //        mat12.SetTexture("assets/textures/extracted_textures/normal_rocks.png.png", "normal");
    //        mat12.SetMetallicRoughness(0.0, 0.78);
    //        mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat12, matLayout));
    //        
    //        graphics::resources::object::Material mat13;
    //        mat13.SetTexture("assets/textures/extracted_textures/diffuse_cave_stones.jpg.png", "albedo");
    //        mat13.SetTexture("assets/textures/extracted_textures/normal_cave_stones.png.png", "normal");
    //        mat13.SetMetallicRoughness(0.0, 0.70);
    //        mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat13, matLayout));
    //        
    //        graphics::resources::object::Material mat14;
    //        mat14.SetTexture("assets/textures/extracted_textures/diffuse_skeleton.jpg.png", "albedo");
    //        mat14.SetTexture("assets/textures/extracted_textures/normal_skeleton.png.png", "normal");
    //        mat14.SetMetallicRoughness(0.0, 0.85);
    //        mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat14, matLayout));
    //        
    //        graphics::resources::object::Material mat15;
    //        mat15.SetTexture("assets/textures/extracted_textures/diffus_fetich.jpg.png", "albedo");
    //        mat15.SetTexture("assets/textures/extracted_textures/normal_fetich.png.png", "normal");
    //        mat15.SetMetallicRoughness(0.0, 0.68);
    //        mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat15, matLayout));
    //        
    //        graphics::resources::object::Material mat16;
    //        mat16.SetTexture("assets/textures/extracted_textures/diffus_grass-transition_clairiere.jpg.png", "albedo");
    //        mat16.SetMetallicRoughness(0.0, 0.86);
    //        mesh->materials.push_back(loaders::MaterialLoader::Load(device, mat16, matLayout));

    //        vikingRoomMesh = mesh;
    //    });

    //loader.ProcessJobs();
    //loader.PurgeFinishedJobs();


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

    timeline::Light light1;
    light1.temp_property.position = glm::vec3(3.0f * glm::cos(time), 4.0f, 3.0f * glm::sin(time));
    light1.temp_property.color = glm::vec3(1.0f, 0.9f, 0.2f);
    light1.temp_property.intensity = 10.0f;
    light1.temp_property.radius = 0.1f;
    light1.temp_property.enabled = true;
    EntityID lightID = app.m_scene->CreateEntity<timeline::Light>(light1);
    auto& light = app.m_scene->GetEntityComponent<timeline::Light>(lightID);

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

        systemManager.Update(*app.m_scene);
        renderer.SetCamera(camera.GetView(), camera.projection);


#ifndef VRAKTAL_EDITOR
        renderer.Render(device.GetSwapchainImage(imageIndex), ImageLayout::Present);
#else
        imGuiWindows.GetContext()->PrepareForDrawing();
        auto image = imGuiWindows.GetContext()->GetViewportImage();
        //On doit ajuster la camera
        if (imGuiWindows.GetContext()->GetViewportState()->width > 0 && imGuiWindows.GetContext()->GetViewportState()->height > 0)
                renderer.SetCamera(camera.GetView(), imGuiWindows.GetContext()->GetViewportProjection());

        //On doit ajuster la camera
        if (imGuiWindows.GetContext()->GetViewportState()->width > 0 && imGuiWindows.GetContext()->GetViewportState()->height > 0)
            renderer.SetCamera(camera.GetView(), imGuiWindows.GetContext()->GetViewportProjection());

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