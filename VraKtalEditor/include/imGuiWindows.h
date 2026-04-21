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
#include <glm/fwd.hpp>
#include <vector>

#include "windows/ImguiWindowBase.h"

#pragma region ForwardDeclarations
class ContentDrawer;
class WindowInput;
class WindowViewport;
class WindowHierarchy;
class Scene;
class ImGuizmoHelper;

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
    void DrawImGui();
    command::CommandHistory* GetCommandHistory() { return m_commandHistory.get(); }
    core::gpu::ImguiContext* GetContext();
    core::Window* GetWindow() { return m_window; };

    bool BeginWindow(const std::string& name, bool defaultStateIfNotExists = true, ImGuiWindowFlags flags = 0);
    void EndWindow(const std::string& name);
    void DisplayWindowStateManagerMenu();

    //helper
    glm::mat4 GetView();
    ImGuizmoHelper* GetImGuizmoHelper() { return m_imGuizmoHelper; };

private:
    void AddWindowToManager(const std::string& name, bool windowState);

    void SetMenuBar();
    void ContentDrawerWindow();
    void MainWindow();
    void LoadProject();

    std::unordered_map<std::string, WindowState> m_windowStatesList;

    core::Window* m_window;
    NewProjectModal m_newProjectModal;
    std::unique_ptr<command::CommandHistory> m_commandHistory;

    core::gpu::ImguiContext* m_imGuiContext;
    graphics::Renderer* m_renderer;
    Scene* m_scene;
    
    std::vector<ImguiWindowBase*> m_windows;

#pragma region windows
    ContentDrawer*  m_contentDrawer;
    WindowInput*    m_windowInput;
    WindowViewport* m_windowViewport;
    WindowHierarchy* m_windowHierarchy;
    ImGuizmoHelper* m_imGuizmoHelper;
#pragma endregion


};

#endif //EDITOR_IMGUIWINDOWS_H