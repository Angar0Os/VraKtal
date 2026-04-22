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