#include "../../../include/windows/others/rightClick.h"
#include "../../../include/imGuiWindows.h"

#include <scene/scene.h>
#include <utils/denseStorage.h>

#include <imgui/imgui.h>
#include <scene/timeline/components/mesh.h>

#include <variant>


RightClick::RightClick(ImGuiWindows* _windows) : m_windows(_windows) {}
RightClick::~RightClick(){}

#pragma region Hierarchy
template<>
void RightClick::Content(WindowHierarchy* _window)
{
    if (ImGui::Button("Add Entity"))
    {
        m_windows->GetScene()->CreateEntity();
        CloseMenu();
    }
}

template<>
void RightClick::Content(EntityID* _ID) 
{
    EntityID selectedEntity = m_windows->IsSelectedItemType<EntityID>() ? m_windows->GetSelectedItem<EntityID>() : INVALID_ENTITY;

    if (selectedEntity == INVALID_ENTITY)
    {
        return;
    }

    if (ImGui::Button("Destroy Entity"))
    {
        if (selectedEntity == *_ID)
        {
            m_windows->ResetSelectedItem();
        }
        m_windows->GetScene()->DestroyEntity(*_ID);
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