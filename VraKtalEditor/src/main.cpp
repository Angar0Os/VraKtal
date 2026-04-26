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

        _input.AddAction("SpawnCave");
        _input.BindActionKey({ input::Key::F , input::Key::LEFT_CONTROL }, "SpawnCave");
        _input.BindActionCallback<App, &App::SpawnCave>("SpawnCave", this, input::KeyState::Press);

        m_scene = new Scene();
        m_scene->RegisterComponentStorage<timeline::MeshInstance>();
        m_scene->RegisterComponentStorage<timeline::Light>();

        m_reManager = new RessourceManager(&_device);

        LoadAssetsDebug(_renderer, _device);
        SpawnCave();
    }
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
        m_reManager->LoadRessource<graphics::resources::Mesh>("assets/models/viking_room.obj");
        timeline::MeshInstance timelineMesh;
        timelineMesh.meshID = m_reManager->GetRessourceID<graphics::resources::Mesh>("assets/models/viking_room.obj");
        timelineMesh.temp_properties.transform = glm::rotate(glm::mat4(1), glm::radians(270.0f) , {1,0,0});
        timelineMesh.temp_properties.transform = glm::scale(timelineMesh.temp_properties.transform,glm::vec3(10,10,10));
        m_scene->CreateEntity<timeline::MeshInstance>(timelineMesh);
    }

    void SpawnCave() {
        std::cout << "Spawn Viking Room Action Triggered" << std::endl;
        timeline::MeshInstance timelineMesh;
        timelineMesh.meshID = m_reManager->GetRessourceID<graphics::resources::Mesh>("assets/models/cave.obj");
        timelineMesh.temp_properties.transform = glm::rotate(glm::mat4(1), glm::radians(270.0f), { 1,0,0 });
        timelineMesh.temp_properties.transform = glm::scale(timelineMesh.temp_properties.transform, glm::vec3(10, 10, 10));
        m_scene->CreateEntity<timeline::MeshInstance>(timelineMesh);
    }

    void LoadAssetsDebug(graphics::Renderer& _renderer , core::gpu::Device& _device)
    {

        auto* matLayout = _renderer.GetPass<graphics::GBufferPass>("GBuffer")->GetMaterialLayout();

        auto* mesh = m_reManager->LoadRessource<graphics::resources::Mesh>("assets/models/cave.obj");
        {
            graphics::resources::object::Material Sand;
            Sand.name = "Sand";
            Sand.SetTexture("assets/textures/extracted_textures/diffuse_sand.jpg.png", "albedo");
            Sand.SetTexture("assets/textures/extracted_textures/normal_sand.png.png", "normal");
            Sand.SetMetallicRoughness(0.0, 0.75);
            graphics::resources::object::Material Rocktill;
            Rocktill.name = "Rocktill";
            Rocktill.SetTexture("assets/textures/extracted_textures/diffus_rocktill_02.jpg.png", "albedo");
            Rocktill.SetTexture("assets/textures/extracted_textures/normal_rocktill_02.png.png", "normal");
            Rocktill.SetMetallicRoughness(0.0, 0.85);
            graphics::resources::object::Material watreplant;
            watreplant.name = "watreplant";
            watreplant.SetTexture("assets/textures/extracted_textures/diffus_plane_watreplant.tga.png", "albedo");
            watreplant.SetTexture("assets/textures/extracted_textures/normal_plane_watreplant.png.png", "normal");
            watreplant.SetMetallicRoughness(0.0, 0.65);
            graphics::resources::object::Material foliage;
            foliage.name = "foliage";
            foliage.SetTexture("assets/textures/extracted_textures/diffus_foliage_03_copy.tga.png", "albedo");
            foliage.SetTexture("assets/textures/extracted_textures/normal_foliage_03.png.png", "normal");
            foliage.SetMetallicRoughness(0.0, 0.78);
            graphics::resources::object::Material foliage1;
            foliage1.name = "foliage";
            foliage1.SetTexture("assets/textures/extracted_textures/diffus_foliage_03_copy.tga.png", "albedo");
            foliage1.SetTexture("assets/textures/extracted_textures/normal_foliage_03.png.png", "normal");
            foliage1.SetMetallicRoughness(0.0, 0.78);
            graphics::resources::object::Material CaveFoliage;
            CaveFoliage.name = "Cave Foliage";
            CaveFoliage.SetTexture("assets/textures/extracted_textures/diffuse_cave_foliage.tga.png", "albedo");
            CaveFoliage.SetTexture("assets/textures/extracted_textures/normal_cave_foliage.png.png", "normal");
            CaveFoliage.SetMetallicRoughness(0.0, 0.60);
            graphics::resources::object::Material Cave;
            Cave.name = "Cave";
            Cave.SetTexture("assets/textures/extracted_textures/diffuse_cave.jpg.png", "albedo");
            Cave.SetTexture("assets/textures/extracted_textures/normal_cave.png.png", "normal");
            Cave.SetMetallicRoughness(0.0, 0.72);
            graphics::resources::object::Material grass;
            grass.name = "grass";
            grass.SetTexture("assets/textures/extracted_textures/diffuse_grass.jpg.png", "albedo");
            grass.SetTexture("assets/textures/extracted_textures/normal_grass.png.png", "normal");
            grass.SetMetallicRoughness(0.0, 0.88);
            graphics::resources::object::Material CaveIndoor;
            CaveIndoor.name = "Cave indoor";
            CaveIndoor.SetTexture("assets/textures/extracted_textures/diffuse_cave_indoor.jpg.png", "albedo");
            CaveIndoor.SetTexture("assets/textures/extracted_textures/normal_cave_indoor.png.png", "normal");
            CaveIndoor.SetMetallicRoughness(0.0, 0.82);
            graphics::resources::object::Material CaveBole;
            CaveBole.name = "Cave bole";
            CaveBole.SetTexture("assets/textures/extracted_textures/diffuse_cave_bole.jpg.png", "albedo");
            CaveBole.SetTexture("assets/textures/extracted_textures/normal_cave_bole.png.png", "normal");
            CaveBole.SetMetallicRoughness(0.0, 0.90);
            graphics::resources::object::Material CaveFoliage1;
            CaveFoliage1.name = "Cave foliage";
            CaveFoliage1.SetTexture("assets/textures/extracted_textures/diffuse_cave_foliage.tga.png", "albedo");
            CaveFoliage1.SetTexture("assets/textures/extracted_textures/normal_cave_foliage.png.png", "normal");
            CaveFoliage1.SetMetallicRoughness(0.0, 0.60);
            graphics::resources::object::Material CaveRock;
            CaveRock.name = "Cave rocktill";
            CaveRock.SetTexture("assets/textures/extracted_textures/diffus_rocktill_02.jpg.png", "albedo");
            CaveRock.SetTexture("assets/textures/extracted_textures/normal_rocktill_02.png.png", "normal");
            CaveRock.SetMetallicRoughness(0.0, 0.80);
            graphics::resources::object::Material Rocks;
            Rocks.name = "Rocks";
            Rocks.SetTexture("assets/textures/extracted_textures/diffus_rocks.jpg.png", "albedo");
            Rocks.SetTexture("assets/textures/extracted_textures/normal_rocks.png.png", "normal");
            Rocks.SetMetallicRoughness(0.0, 0.78);
            graphics::resources::object::Material Stones;
            Stones.name = "Stones";
            Stones.SetTexture("assets/textures/extracted_textures/diffuse_cave_stones.jpg.png", "albedo");
            Stones.SetTexture("assets/textures/extracted_textures/normal_cave_stones.png.png", "normal");
            Stones.SetMetallicRoughness(0.0, 0.70);
            graphics::resources::object::Material skeleton;
            skeleton.name = "skeleton";
            skeleton.SetTexture("assets/textures/extracted_textures/diffuse_skeleton.jpg.png", "albedo");
            skeleton.SetTexture("assets/textures/extracted_textures/normal_skeleton.png.png", "normal");
            skeleton.SetMetallicRoughness(0.0, 0.85);
            graphics::resources::object::Material fetich;
            fetich.name = "fetich";
            fetich.SetTexture("assets/textures/extracted_textures/diffus_fetich.jpg.png", "albedo");
            fetich.SetTexture("assets/textures/extracted_textures/normal_fetich.png.png", "normal");
            fetich.SetMetallicRoughness(0.0, 0.68);
            graphics::resources::object::Material GrassTransition;
            GrassTransition.name = "grass transition";
            GrassTransition.SetTexture("assets/textures/extracted_textures/diffus_grass-transition_clairiere.jpg.png", "albedo");
            GrassTransition.SetMetallicRoughness(0.0, 0.86);

            mesh->materials.push_back(loaders::MaterialLoader::Load(_device, GrassTransition  , matLayout)) ;
            mesh->materials.push_back(loaders::MaterialLoader::Load(_device, CaveFoliage   , matLayout))    ;
            mesh->materials.push_back(loaders::MaterialLoader::Load(_device, Rocktill   , matLayout))       ;
            mesh->materials.push_back(loaders::MaterialLoader::Load(_device, Sand, matLayout));
            mesh->materials.push_back(loaders::MaterialLoader::Load(_device, watreplant   , matLayout))     ;
            mesh->materials.push_back(loaders::MaterialLoader::Load(_device, foliage   , matLayout))        ;
            mesh->materials.push_back(loaders::MaterialLoader::Load(_device, foliage1   , matLayout))       ;
            mesh->materials.push_back(loaders::MaterialLoader::Load(_device, Cave   , matLayout))           ;
            mesh->materials.push_back(loaders::MaterialLoader::Load(_device, grass   , matLayout))          ;
            mesh->materials.push_back(loaders::MaterialLoader::Load(_device, CaveIndoor   , matLayout))     ;
            mesh->materials.push_back(loaders::MaterialLoader::Load(_device, CaveBole   , matLayout))       ;
            mesh->materials.push_back(loaders::MaterialLoader::Load(_device, CaveFoliage  , matLayout))     ;
            mesh->materials.push_back(loaders::MaterialLoader::Load(_device, CaveRock  , matLayout))        ;
            mesh->materials.push_back(loaders::MaterialLoader::Load(_device, Rocks  , matLayout))           ;
            mesh->materials.push_back(loaders::MaterialLoader::Load(_device, Stones  , matLayout))          ;
            mesh->materials.push_back(loaders::MaterialLoader::Load(_device, skeleton  , matLayout))        ;
            mesh->materials.push_back(loaders::MaterialLoader::Load(_device, fetich  , matLayout))          ;
            
        }

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
    ImGuiWindows imGuiWindows = ImGuiWindows(device.GetImGuiContext(), &renderer, &window, input , app.m_scene ,*app.m_reManager , &device);
    device.GetImGuiContext()->BindPrepareDrawData([&]()
        {
            imGuiWindows.DrawImGui();
        });
#endif //VRAKTAL_EDITOR

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

        systemManager.Update(*app.m_scene);
        light.temp_property.position = glm::vec3(3.0f * glm::cos(time), 4.0f, 3.0f * glm::sin(time));

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

       
#ifndef VRAKTAL_EDITOR
        renderer.SetCamera(camera.GetView(), camera.projection);
        renderer.Render(device.GetSwapchainImage(imageIndex), ImageLayout::Present);
#else
        imGuiWindows.GetContext()->PrepareForDrawing();
        auto image = imGuiWindows.GetContext()->GetViewportImage();
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