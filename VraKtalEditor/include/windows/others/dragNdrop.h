#pragma once
#include <core/manager/ressourceManager.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

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
    DraggedType* DropWindow(DroppedReceived& object) {

        DraggedType* toReturn = nullptr;
		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
		{
			ImVec2 var = ImGui::GetWindowPos();
			var.x = ImGui::GetWindowSize().x + var.x;
			var.y = ImGui::GetWindowSize().y + var.y;

			ImRect rect(ImGui::GetWindowPos(), var);

            if (ImGui::BeginDragDropTargetCustom(rect, ImGui::GetCurrentWindow()->ID))
			{
                toReturn = Content<DraggedType , DroppedReceived>(object);
                ImGui::EndDragDropTarget();
			}
		}
		return toReturn;
    }

    template<typename DraggedType, typename DroppedReceived>
    DraggedType* DropItem(DroppedReceived& object) {
        DraggedType* droppedEntry = nullptr;
        if (ImGui::BeginDragDropTarget())
        {
            droppedEntry = Content<DraggedType, DroppedReceived>(object);
            ImGui::EndDragDropTarget();
        }
        return droppedEntry;
    }

private:
    ImGuiWindows& m_windowManager;
    RessourceManager& m_reManager;

    template<typename DraggedType, typename DroppedReceived>
    DraggedType* Content(DroppedReceived& object) {
        ImGui::BeginPopup("DropAvailability");
        ImGui::Text("No DropAvailable");
        ImGui::EndPopup();
        return nullptr;
    }

};

template<>
void DragNDrop::Drag(FileEntry& _fileEntry);

template<>
FileEntry* DragNDrop::Content(FileEntry& _fileEntry);

template<>
FileEntry* DragNDrop::Content(Mesh_ID& _meshID);

using MaterialIndex = uint32_t;
template<>
void DragNDrop::Drag(MaterialIndex& _materialIndex);
template<>
MaterialIndex* DragNDrop::Content(MaterialIndex& _meshID);