#pragma once
#include <cstdint>
#include <utils/macro.h>
#include <memory>
#include "core/enum.h"

class SystemManager;
class RessourceManager;
class SceneManager;
class AssetManager;
class Scene;

namespace core
{
    class Window;
    class Time;
    namespace gpu
    {
        class Device;
        class Image;
    }
    class Input;
}
namespace graphics
{
    class Renderer;
}

struct Core;
struct Graphics;
struct ECS;

struct FrameStats;

class Vraktal
{
public:
    Vraktal(uint32_t _width, uint32_t _height, const char* _title);
	~Vraktal();

    void Update();
    bool BeginFrame();
    void Render();
    void RenderInImage(core::gpu::Image* _image, core::ImageLayout _layout);
    void EndFrame();

    bool ShouldClose() const;    
    void Cleanup();

private:

    std::unique_ptr<Core> m_core;
    std::unique_ptr<Graphics> m_graphics;
    std::unique_ptr<ECS> m_ecs;

    float       time = 0.0f;
    const float timeStep = (1.0f / 240.0f) / 5.0f;
    uint32_t currentFrameIndex = 0;
    uint32_t frameCounter = 0;
    uint32_t m_imageIndex = 0;
   
public:
    VRAKTAL_DECLARE_GETTER_REF(core::Window      , Window)
    VRAKTAL_DECLARE_GETTER_REF(core::gpu::Device, Device)
    VRAKTAL_DECLARE_GETTER_REF(core::Input       , Input)
    VRAKTAL_DECLARE_GETTER_REF(graphics::Renderer, Renderer)
    VRAKTAL_DECLARE_GETTER_REF(SystemManager, SystemManager)
    VRAKTAL_DECLARE_GETTER_REF(AssetManager, AssetManager)
    VRAKTAL_DECLARE_GETTER_REF(RessourceManager, RessourceManager)
    VRAKTAL_DECLARE_GETTER_CPY(uint32_t, ImageIndex)
    VRAKTAL_DECLARE_GETTER_REF(SceneManager, SceneManager)
    VRAKTAL_DECLARE_GETTER_REF(core::Time, Time)
};