#ifndef EDITOR_IMGUIWINDOWS_H
#define EDITOR_IMGUIWINDOWS_H
#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <contentDrawer.h>
#include <newProjectModal.h>

#include <core/window.h>
#include <core/gpu/device.h>

#include <command/commandHistory.h>

#include <demo/scene.h>

#include <loaders/meshLoader.h>

#include <imGuizmo/ImGuizmo.h>

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
        std::vector<demo::Scene>* scenes,
        loaders::MeshLoader* meshLoader);
    ~ImGuiWindows();

    void PrepareImGuiWindows();

    command::CommandHistory* GetCommandHistory() { return m_commandHistory.get(); }
    core::gpu::ImguiContext* GetContext();

    demo::Scene* GetActiveScene() const;

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

    void EditTransformByIndice(const float* cameraView, const float* cameraProjection, int objIndice);

    std::optional<size_t>   m_selectedObjectIndex;
    std::optional<size_t>   m_activeSceneIndex;
    std::optional<size_t>   m_editingSceneIndex;        
    std::optional<size_t>   m_pendingDeleteSceneIndex;

    bool                m_gizmoEnabled = true;
    ImGuizmo::OPERATION m_gizmoOperation = ImGuizmo::TRANSLATE;

    core::gpu::Device* m_device = nullptr;

    std::vector<demo::Scene>* m_scenes;
    loaders::MeshLoader* m_meshLoader;

    ContentDrawer           m_contentDrawer;
    core::Window* m_window;
    NewProjectModal         m_newProjectModal;

    std::unique_ptr<command::CommandHistory> m_commandHistory;
    core::gpu::ImguiContext* m_imGuiContext;
    graphics::Renderer* m_renderer;
};

#endif // EDITOR_IMGUIWINDOWS_H