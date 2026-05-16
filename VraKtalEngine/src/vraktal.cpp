#include <vraktal.h>

#include <core/window.h>
#include <core/time.h>
#include <core/gpu/device.h>
#include <core/input/input.h>

#include <core/manager/assetManager.h>
#include <core/manager/ressourceManager.h>

#include <scene/scene.h>

#include <graphics/renderer.h>

#include <scene/system/systemManager.h>
#include <core/manager/sceneManager.h>

#include <utils/macro.h>

#include <cstdint>
#include <string>

#include <scene/system/systems/meshSystem.h>
#include <scene/system/systems/lightSystem.h>
#include <core/gpu/image.h>

struct Core
{
    Core(uint32_t _width, uint32_t _height, const char* _title) : window(_width, _height, _title), device(window), input(&window, &device){}
    core::Window        window;
    core::gpu::Device   device;
    core::Input         input;
    core::Time          time;
};

struct Graphics
{
    Graphics(Vraktal& _vraktal) : renderer(_vraktal.GetWindow(), _vraktal.GetDevice() , ressourceManager ,_vraktal.GetTime()), ressourceManager(_vraktal.GetDevice() , renderer) {}
    graphics::Renderer renderer;
    AssetManager       assetManager;
    RessourceManager   ressourceManager;
};

struct ECS
{
    ECS(Vraktal& _vraktal) : systemManager(), sceneManager()
    {
        systemManager.AddSystem<MeshSystem>(_vraktal.GetRenderer(), _vraktal.GetRessourceManager());
    };
    SystemManager systemManager;
    SceneManager sceneManager;
};

Vraktal::Vraktal(uint32_t _width, uint32_t _height, const char* _title)
{
    m_core = std::make_unique<Core>(_width, _height, _title);
    m_graphics = std::make_unique<Graphics>(*this);
    m_ecs = std::make_unique<ECS>(*this);
}

Vraktal::~Vraktal() {}

void Vraktal::Update()
{
    GetTime().Begin("Engine", "Update");
    GetTime().Update();

    time += timeStep;

    if (m_core->device.NeedsResize())
    {
        m_core->device.RecreateSwapchain();
        m_graphics->renderer.OnResize();
        m_core->device.ClearResizeFlag();
        return;
    }

    m_core.get()->time.Begin("Engine","Inputs/PollEvents");
    m_core->window.PollEvents();
    m_core->input.Update();
    m_core.get()->time.End("Engine","Inputs/PollEvents");

    m_core.get()->time.Begin("Engine","Scene Update");
    for(Scene& _scene : m_ecs.get()->sceneManager.GetScenes())
    {
        m_ecs.get()->systemManager.Update(_scene);
    }
    m_core.get()->time.End("Engine","Scene Update");
    GetTime().End("Engine", "Update");
}

bool Vraktal::BeginFrame()
{
    m_imageIndex = m_core->device.AcquireNextImage(currentFrameIndex);

    if (m_imageIndex == UINT32_MAX)
        return false;

    return true;
}

void Vraktal::Render()
{
    GetRenderer().Render(GetDevice().GetSwapchainImage(m_imageIndex), ImageLayout::Present);
}

void Vraktal::RenderInImage(core::gpu::Image* _image, ImageLayout _layout) {
    GetRenderer().Render(_image, _layout);
}

void Vraktal::EndFrame()
{
    m_graphics->renderer.Advance();

    m_core->device.Present(m_imageIndex, currentFrameIndex);

    currentFrameIndex =
        (currentFrameIndex + 1) % core::gpu::Device::s_FRAMES_IN_FLIGHT;

    frameCounter++;
}

bool Vraktal::ShouldClose() const
{
    return m_core->window.ShouldClose();
}

void Vraktal::Cleanup()
{
    m_core->device.WaitIdle();
    m_graphics->renderer.Cleanup();
}

VRAKTAL_DEFINE_GETTER_REF(core::Window,         Window ,        m_core->window , Vraktal)
VRAKTAL_DEFINE_GETTER_REF(core::gpu::Device,    Device ,        m_core->device, Vraktal)
VRAKTAL_DEFINE_GETTER_REF(core::Input,          Input,          m_core->input, Vraktal)
VRAKTAL_DEFINE_GETTER_REF(graphics::Renderer,   Renderer,       m_graphics->renderer, Vraktal)
VRAKTAL_DEFINE_GETTER_REF(SystemManager,        SystemManager,  m_ecs->systemManager, Vraktal)
VRAKTAL_DEFINE_GETTER_REF(AssetManager,         AssetManager,   m_graphics->assetManager, Vraktal)
VRAKTAL_DEFINE_GETTER_REF(RessourceManager,     RessourceManager,m_graphics->ressourceManager, Vraktal)
VRAKTAL_DEFINE_GETTER_CPY(uint32_t, ImageIndex , m_imageIndex, Vraktal)
VRAKTAL_DEFINE_GETTER_REF(SceneManager, SceneManager , m_ecs->sceneManager, Vraktal)
VRAKTAL_DEFINE_GETTER_REF(core::Time, Time, m_core->time, Vraktal)