#ifndef EDITOR_IMGUIWINDOWS_H
#define EDITOR_IMGUIWINDOWS_H
#pragma once

#include <memory>
#include <optional>
#include "contentDrawer.h"
#include <core/window.h>
#include "newProjectModal.h"
#include "command/commandHistory.h"
#include <demo/scene.h>
#include <loaders/meshLoader.h>

namespace core::gpu {
    class ImguiContext;
}

namespace graphics {
    class Renderer;
}

class ImGuiWindows
{
public:
    ImGuiWindows(core::gpu::ImguiContext* _imGuiContext,
        graphics::Renderer* _renderer,
        core::Window* window,
        demo::Scene* scene,
        loaders::MeshLoader* meshLoader);
    ~ImGuiWindows();

    void PrepareImGuiWindows();

    command::CommandHistory* GetCommandHistory() { return m_commandHistory.get(); }
    core::gpu::ImguiContext* GetContext();

private:
    void SetMenuBar();
    void ContentDrawerWindow();
    void HierarchyWindow();
    void InspectorWindow();
    void MainWindow();
    void TestWindow();
    void Viewport();

    void InspectorTransform(size_t index);
    void InspectorCameraProperties(size_t index);
    void InspectorLightProperties(size_t index);

    void LoadProject();
    void EditTransformByIndice(const float* cameraView, const float* cameraProjection, int objIndice);

    std::optional<size_t>   m_selectedObjectIndex;  

    demo::Scene* m_scene;
    loaders::MeshLoader* m_meshLoader;

    ContentDrawer           m_contentDrawer;
    core::Window* m_window;
    NewProjectModal         m_newProjectModal;

    std::unique_ptr<command::CommandHistory> m_commandHistory;
    core::gpu::ImguiContext* m_imGuiContext;
    graphics::Renderer* m_renderer;
};

#endif // EDITOR_IMGUIWINDOWS_H