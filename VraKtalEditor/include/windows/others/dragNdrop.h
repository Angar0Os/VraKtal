#pragma once
#include <core/manager/ressourceManager.h>

#include <imgui/imgui.h>

namespace graphics::resources
{
    class Mesh;
}

namespace scene::timeline::components
{
    class Light;
    class Mesh;
}

struct FileEntry;
class ImGuiWindows;
class RessourceManager;

using Mesh_ID = uint32_t;
static constexpr Mesh_ID INVALID_ID = 0xFFFFFFFFu;

struct DragNDrop
{
    DragNDrop(ImGuiWindows& _windowManager , RessourceManager& _reManager): m_windowManager(_windowManager) , m_reManager(_reManager){};

    template<typename T>
    void Drag(T& object) {
        ImGui::BeginPopup("DragAvailability");
        ImGui::Text("No DragAvailable");
        ImGui::EndPopup();
    };
    
    template<typename DraggedType , typename DroppedReceived>
    DraggedType* Drop(DroppedReceived& object) {
        ImGui::BeginPopup("DropAvailability");
        ImGui::Text("No DropAvailable");
        ImGui::EndPopup();
    }

private:
    ImGuiWindows& m_windowManager;
    RessourceManager& m_reManager;
};

template<>
void DragNDrop::Drag(FileEntry& _fileEntry);

template<>
FileEntry* DragNDrop::Drop(FileEntry& _fileEntry);

template<>
FileEntry* DragNDrop::Drop(Mesh_ID& _meshID);