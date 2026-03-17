#ifndef EDITOR_IMGUIWINDOWS_H
#define EDITOR_IMGUIWINDOWS_H
#pragma once

#include <memory>
#include "contentDrawer.h"
#include <core/window.h>
#include "newProjectModal.h"
#include "command/commandHistory.h"

namespace core::gpu {
    class ImguiContext;
}

namespace graphics {
    class Renderer;
}

class ImGuiWindows
{
public:
    ImGuiWindows(core::gpu::ImguiContext* _imGuiContext, graphics::Renderer* _renderer, core::Window* window);
    ~ImGuiWindows();
    void PrepareImGuiWindows();
    command::CommandHistory* GetCommandHistory() { return m_commandHistory.get(); }

private:
    void SetMenuBar();
    void ContentDrawerWindow();
    void HierarchyWindow();
    void mainWindow();
    void testWindow();
    void Viewport();
    void EditTransformByIndice(const float* cameraView, const float* cameraProjection, int objIndice);
    void LoadProject();

    ContentDrawer m_contentDrawer;
    core::Window* m_window;
    NewProjectModal m_newProjectModal;
    std::unique_ptr<command::CommandHistory> m_commandHistory;

    core::gpu::ImguiContext* m_imGuiContext;
    graphics::Renderer* m_renderer;
};

#endif //EDITOR_IMGUIWINDOWS_H