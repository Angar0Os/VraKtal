#include "../../../include/windows/popup/rightClick.h"
#include "../../../include/imGuiWindows.h"
#include "../../../include/windows/windowHierarchy.h"

#include <scene/scene.h>
#include <utils/denseStorage.h>

#include <imgui/imgui.h>
#include <scene/timeline/components/mesh.h>
#include <scene/timeline/components/light.h>

#include <variant>
#include <unordered_map>
#include <windows/others/folder.h>


RightClick::RightClick(ImGuiWindows* _windows) : m_windows(_windows) {}
RightClick::~RightClick(){}

#pragma region Hierarchy
template<>
void RightClick::Content(WindowHierarchy* _window)
{
    if (ImGui::Selectable("Add Entity"))
    {
        m_windows->GetScene()->CreateEntity();
        CloseMenu();
    }
    if (ImGui::Selectable("Add Folder"))
    {
        _window->CreateFolder("New Folder");
        CloseMenu();
    }
}
template<>
void RightClick::Content(WindowHierarchy* window, hierarchy::Folder* folder)
{
    if (ImGui::Selectable("Rename Folder"))
    {
        window->RenameFolder(folder->id);
        CloseMenu();
    }

    if (ImGui::Selectable("Delete Folder"))
    {
        window->DeleteFolder(folder->id);
        CloseMenu();
    }

    if (ImGui::Selectable("Delete Folder And Content"))
    {
        window->DeleteFolderAndContent(folder->id);
        CloseMenu();
    }
}


template<>
void RightClick::Content(EntityID* _ID) 
{
    if (*_ID == INVALID_ENTITY)
    {
        return;
    }

    if (ImGui::Selectable("Destroy Entity"))
    {
        m_windows->GetScene()->DestroyEntity(*_ID);
        CloseMenu();
    }
}

template<>
void RightClick::Content(std::vector<EntityID>* _IdMap)
{
    if (ImGui::Selectable("Destroy Entity"))
    {
        for (auto var : *_IdMap)
        {
            m_windows->GetScene()->DestroyEntity(var);
        }
        CloseMenu();
    }
}

#pragma endregion

#pragma region Inspector

template<>
void RightClick::Content(WindowInspector* _window)
{
    EntityID selectedEntity = m_windows->IsSelectedItemType<EntityID>() ? m_windows->GetSelectedItem<EntityID>() : INVALID_ENTITY;

    if (selectedEntity == INVALID_ENTITY)
        return;


    bool bClickedComp = false;

    ImGui::Text("Add Component");
    
    if (!m_windows->GetScene()->GetComponentStorage<timeline::MeshInstance>().Has(selectedEntity))
    {
        if (ImGui::Button("Mesh"))
        {
            timeline::MeshInstance mesh;
            m_windows->GetScene()->GetComponentStorage<timeline::MeshInstance>().Add(selectedEntity , mesh);
            bClickedComp = true;
        }
    }

    if (!m_windows->GetScene()->GetComponentStorage<timeline::Light>().Has(selectedEntity))
    {
        if (ImGui::Button("Light"))
        {
            timeline::Light light;
            m_windows->GetScene()->GetComponentStorage<timeline::Light>().Add(selectedEntity, light);
            bClickedComp = true;
        }
    }

    if (bClickedComp)
    {
        CloseMenu();
    }
}

#pragma endregion