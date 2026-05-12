#ifndef EDITOR_IMGUIWINDOWS_H
#define EDITOR_IMGUIWINDOWS_H
#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <core/window.h>
#include "command/commandHistory.h"
#include <imgui/imgui.h>
#include <vector>
#include <variant>
#include <glm/glm.hpp>

#include "windows/ImguiWindowBase.h"
#include <scene/timeline/entityBase.h>
#include <utility>

#pragma region ForwardDeclarations
//Windows
class ContentDrawer;
class WindowInput;
class WindowViewport;
class WindowHierarchy;
class ProjectModal;

//Others
class   ImGuizmoHelper;
class   MeshPlot;
struct  DragNDrop;
struct  Inspect;
struct  FileEntry;

struct ImguiOthers;

//Popups
class   RightClick;
class   ProjectModal;

struct Popups;



//Engine
class RessourceManager;
class AssetManager;
class Scene;


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
    namespace resources {
        struct Mesh;
    }
    class Renderer;
}

namespace core::gpu {
    class Device;
}

namespace hierarchy {
    struct Folder;
}

#pragma endregion
class ImGuiWindows
{
public:

    ImGuiWindows(core::gpu::ImguiContext* _imGuiContext, graphics::Renderer* _renderer, core::Window* window, core::Input& _input, Scene* _scene, RessourceManager& _manager , AssetManager& _astManager, core::gpu::Device* _device);
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
    core::Input* GetInput() { return m_input; };

    void ResetSelectedItem();

    void HandleExternalFileDrop(const std::vector<std::string>& filePaths);

    template<typename T>
    void SetSelectedItem(T value)
    {
        m_selectedItem = value;
    }


    template<typename T>
    bool IsSelectedItemType() const
    {
        return std::holds_alternative<T>(m_selectedItem);
    }

    template<typename T>
    T& GetSelectedItem()
    {
        if (IsSelectedItemType<T>())
            return std::get<T>(m_selectedItem);
        throw std::bad_variant_access();
    }

    bool BeginWindow(const std::string& name, bool defaultStateIfNotExists = true, ImGuiWindowFlags flags = 0);
    void EndWindow(const std::string& name);
    void DisplayWindowStateManagerMenu();



    //helper
    glm::mat4 GetView();

    //Get Others
    ImGuizmoHelper* GetImGuizmoHelper();
    MeshPlot* GetMeshPlot();
    DragNDrop* GetDragNDrop();
    Inspect* GetInspect();
    
    //Get popups
    RightClick* GetRightClick();
    ProjectModal* GetProjectModal();

private:
    void AddWindowToManager(const std::string& name, bool windowState);

    void SetMenuBar();
    void ContentDrawerWindow();
    void MainWindow();
    void LoadProject();

    std::unordered_map<std::string, WindowState> m_windowStatesList;

    std::unique_ptr<command::CommandHistory> m_commandHistory;

#pragma region windows
    ContentDrawer* m_contentDrawer;
    std::vector<ImguiWindowBase*> m_windows;
#pragma endregion

    ImguiOthers* m_others;
    Popups* m_popups;

    core::Window* m_window;
    graphics::Renderer* m_renderer;
    core::gpu::ImguiContext* m_imGuiContext;
    Scene* m_scene;
    core::Input* m_input;
    std::variant<std::monostate, EntityID, graphics::resources::Mesh*, FileEntry* , hierarchy::Folder*> m_selectedItem = std::monostate{};
};

#endif //EDITOR_IMGUIWINDOWS_H