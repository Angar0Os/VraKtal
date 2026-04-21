#ifndef EDITOR_IMGUIWINDOWS_H
#define EDITOR_IMGUIWINDOWS_H
#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include "contentDrawer.h"
#include <core/window.h>
#include "newProjectModal.h"
#include "command/commandHistory.h"
#include <imgui/imgui.h>

#pragma region ForwardDeclarations
class ContentDrawer;
class WindowInput;
class WindowViewport;
class WindowHierarchy;
class Scene;

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
    ImGuiWindows(core::gpu::ImguiContext* _imGuiContext, graphics::Renderer* _renderer, core::Window* window , core::Input& _input , Scene* _scene);
   
    struct WindowState 
    {
        bool isOpen = true;
        bool keepOpen = true;
    };

    ~ImGuiWindows();
    void PrepareImGuiWindows();
    command::CommandHistory* GetCommandHistory() { return m_commandHistory.get(); }
    core::gpu::ImguiContext* GetContext();
    core::Window* GetWindow() { return m_window; };

    bool BeginWindow(const std::string& name, bool defaultStateIfNotExists = true, ImGuiWindowFlags flags = 0);
    void EndWindow(const std::string& name);
    void DisplayWindowStateManagerMenu();

private:
    void AddWindowToManager(const std::string& name, bool windowState);

    void SetMenuBar();
    void ContentDrawerWindow();
    void MainWindow();
    void testWindow();
    void EditTransformByIndice(const float* cameraView, const float* cameraProjection, int objIndice);
    void LoadProject();
    void ViewportWindow();

    std::unordered_map<std::string, WindowState> m_windowStatesList;

    core::Window* m_window;
    NewProjectModal m_newProjectModal;
    std::unique_ptr<command::CommandHistory> m_commandHistory;

    core::gpu::ImguiContext* m_imGuiContext;
    graphics::Renderer* m_renderer;
    Scene* m_scene;
    
#pragma region windows
    ContentDrawer*  m_contentDrawer;
    WindowInput*    m_windowInput;
    WindowViewport* m_windowViewport;
    WindowHierarchy* m_windowHierarchy;
#pragma endregion


};

#endif //EDITOR_IMGUIWINDOWS_H