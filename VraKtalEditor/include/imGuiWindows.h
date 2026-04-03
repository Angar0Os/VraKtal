#ifndef EDITOR_IMGUIWINDOWS_H
#define EDITOR_IMGUIWINDOWS_H
#pragma once

#include <memory>
#include <core/window.h>
#include "newProjectModal.h"
#include "command/commandHistory.h"

#pragma region ForwardDeclarations
class ContentDrawer;
class WindowInput;
class WindowViewport;

namespace core {
    class Input;

    namespace gpu {
        class ImguiContext;
    }
}
namespace graphics {
    class Renderer;
}

#pragma endregion
class ImGuiWindows
{
public:
    ImGuiWindows(core::gpu::ImguiContext* _imGuiContext, graphics::Renderer* _renderer, core::Window* window , core::Input& _input);
    ~ImGuiWindows();
    void PrepareImGuiWindows();
    command::CommandHistory* GetCommandHistory() { return m_commandHistory.get(); }
    core::gpu::ImguiContext* GetContext();
    core::Window* GetWindow() { return m_window; };
private:
    void SetMenuBar();
    void ContentDrawerWindow();
    void HierarchyWindow();
    void mainWindow();
    void testWindow();
    void EditTransformByIndice(const float* cameraView, const float* cameraProjection, int objIndice);
    void LoadProject();

    core::Window* m_window;
    NewProjectModal m_newProjectModal;
    std::unique_ptr<command::CommandHistory> m_commandHistory;

    core::gpu::ImguiContext* m_imGuiContext;
    graphics::Renderer* m_renderer;
    
#pragma region windows
    ContentDrawer*  m_contentDrawer;
    WindowInput*    m_windowInput;
    WindowViewport* m_windowViewport;
#pragma endregion


};

#endif //EDITOR_IMGUIWINDOWS_H