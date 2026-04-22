#ifndef EDITOR_IMGUIWINDOWS_H
#define EDITOR_IMGUIWINDOWS_H
#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <core/window.h>
#include "newProjectModal.h"
#include "command/commandHistory.h"
#include <imgui/imgui.h>
#include <vector>
#include <variant>
#include <glm/glm.hpp>

#include "windows/ImguiWindowBase.h"
#include <scene/timeline/entityBase.h>
#include <windows/others/inspector.h>

#pragma region ForwardDeclarations
class ContentDrawer;
class WindowInput;
class WindowViewport;
class WindowHierarchy;
class Scene;
class ImGuizmoHelper;
class RessourceManager;

namespace core {
    class Input;

    namespace gpu {
        class ImguiContext;
    }
}

namespace timeline {
    struct MeshInstance;
}

namespace graphics {
    class Renderer;
}
#pragma endregion
class ImGuiWindows
{
public:
    
    ImGuiWindows(core::gpu::ImguiContext* _imGuiContext, graphics::Renderer* _renderer, core::Window* window , core::Input& _input , Scene* _scene , RessourceManager& _manager);
    ~ImGuiWindows();

    struct WindowState 
    {
        bool isOpen = true;
        bool keepOpen = true;
    };

    void DrawImGui();
    command::CommandHistory* GetCommandHistory() { return m_commandHistory.get(); }
    core::gpu::ImguiContext* GetContext();
    core::Window* GetWindow() { return m_window; };
    Scene* GetScene() { return m_scene; };
    Inspect* GetInspect() { return m_inspect;  };


    bool BeginWindow(const std::string& name, bool defaultStateIfNotExists = true, ImGuiWindowFlags flags = 0);
    void EndWindow(const std::string& name);
    void DisplayWindowStateManagerMenu();

    EntityID selectedItem;

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

    NewProjectModal m_newProjectModal;
    std::unique_ptr<command::CommandHistory> m_commandHistory;

    Scene* m_scene;

#pragma region windows
    ContentDrawer*  m_contentDrawer;
    ImGuizmoHelper* m_imGuizmoHelper;
    std::vector<ImguiWindowBase*> m_windows;
#pragma endregion

    core::Window* m_window;
    graphics::Renderer* m_renderer;
    core::gpu::ImguiContext* m_imGuiContext;
    Inspect* m_inspect;
    core::Input* m_input;
};

#endif //EDITOR_IMGUIWINDOWS_H