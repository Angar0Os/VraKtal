#include "./windows/windowHierarchy.h"
#include "../../include/imGuiWindows.h"

#include "../../include/windows/others/imGuizmoHelper.h"
#include "../../include/windows/others/dragNdrop.h"

#include "../../include/windows/popup/rightClick.h"

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


#include <scene/scene.h>
#include <scene/timeline/components/mesh.h>
#include <graphics/renderer.h>
#include <core/gpu/buffer.h>

WindowHierarchy::WindowHierarchy(Scene& _scene, ImGuiWindows& _imGuiWindows) : m_scene(_scene), m_imGuiWindows(_imGuiWindows), m_renamingEntity(INVALID_ENTITY), m_rangeSelectStartEnd({INVALID_ENTITY , INVALID_ENTITY})
{
}

WindowHierarchy::~WindowHierarchy()
{
}

void WindowHierarchy::Draw()
{
    ComponentStorage<timeline::MeshInstance>& meshStorage = m_scene.GetComponentStorage<timeline::MeshInstance>();
    if (m_imGuiWindows.BeginWindow("Hierarchy", true))
    {
        if (m_rangeSelectStartEnd.first != INVALID_ENTITY && m_rangeSelectStartEnd.second != INVALID_ENTITY)
        {
            SetSelectedEntityInRange(m_rangeSelectStartEnd.first, m_rangeSelectStartEnd.second);
            m_rangeSelectStartEnd.first = INVALID_ENTITY;
            m_rangeSelectStartEnd.second= INVALID_ENTITY;
        }

        for (size_t i = 0; i < m_scene.GetAliveEntities().size() ; i++)
        {
            DrawEntityHierarchyItem(i);
        }

        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) &&
            (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) &&
            !ImGui::IsAnyItemHovered())
        {
            std::cout << "Cleared Entity" << std::endl;
            m_entitiesSelected.clear();
        }

        if (m_entitiesSelected.size() == 0)
        {   
            m_imGuiWindows.GetRightClick()->Draw<WindowHierarchy>(this);
        }
        else if (m_entitiesSelected.size() == 1)
        {
            size_t id = m_entitiesSelected.begin()->first;
            if (id < m_scene.GetAliveEntities().size())
            {
                m_imGuiWindows.GetRightClick()->Draw<EntityID>(&m_imGuiWindows.GetScene()->GetAliveEntities()[id]);
            }
        }
        else
        {
            std::vector<EntityID> selectedEntities = ConstructSelectedEntitiesVector();
            m_imGuiWindows.GetRightClick()->Draw<std::vector<EntityID>>(&selectedEntities);
        }
    }
    m_imGuiWindows.EndWindow("Hierarchy");
}

void WindowHierarchy::DrawEntityHierarchyItem(size_t indexInAlive)
{

    EntityID ID = m_scene.GetAliveEntities()[indexInAlive];
    if (!m_scene.GetComponentStorage<std::string>().Has(ID))
        return;

    std::string& label = m_scene.GetEntityComponent<std::string>(ID);

    EntityID SelectedItemID = m_imGuiWindows.IsSelectedItemType<EntityID>() ? m_imGuiWindows.GetSelectedItem<EntityID>() : INVALID_ENTITY;
    bool isEntitySelected = false;

    if (m_entitiesSelected.contains(indexInAlive))
    {
        isEntitySelected = m_entitiesSelected.at(indexInAlive);
    }

    if (m_renamingEntity == indexInAlive)
    {
        ImGui::SetNextItemWidth(-1.0f);
    
        const bool enterPressed = ImGui::InputText(
            "##RenameEntity",
            m_entityRenameBuffer,
            sizeof(m_entityRenameBuffer),
            ImGuiInputTextFlags_EnterReturnsTrue |
            ImGuiInputTextFlags_AutoSelectAll
        );
    
        if (ImGui::IsItemActive() && ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            m_renamingEntity = -1;
        }
        else if (enterPressed)
        {
            label = m_entityRenameBuffer;
            m_renamingEntity = -1;
        }
        else if (ImGui::IsItemDeactivatedAfterEdit())
        {
            label = m_entityRenameBuffer;
            m_renamingEntity = -1;
        }
        else if (SelectedItemID != m_renamingEntity)
        {
            m_renamingEntity = -1;
        }
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.4f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.5f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.1f, 0.3f, 0.9f, 1.0f));
    
        ImGui::Selectable(label.c_str(), isEntitySelected);
    
        ImGui::PopStyleColor(3);
    
        if (ImGui::IsItemHovered())
        {
            //si on hover one entitee on la select avant
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            {
                if (!m_entitiesSelected.contains(indexInAlive) || !m_entitiesSelected.at(indexInAlive))
                {
                    SetSelectedEntity(indexInAlive);
                }
            }
            else if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                m_renamingEntity = indexInAlive;
                strncpy_s(m_entityRenameBuffer, sizeof(m_entityRenameBuffer), label.c_str(), _TRUNCATE);
            }
            else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                if (ImGui::GetIO().KeyShift)
                {
                    if (m_rangeSelectStartEnd.first == INVALID_ENTITY)
                    {
                        if (m_entitiesSelected.size() > 0)
                        {
                            size_t Smallest = GetSmallestSelectedEntity();
                            size_t Biggest = GetBiggestSelectedEntity();
                            if (Biggest > indexInAlive && Smallest < indexInAlive)
                            {
                                m_rangeSelectStartEnd.first = Biggest;
                                m_rangeSelectStartEnd.second = Smallest;
                            }
                            if (Biggest > indexInAlive)
                            {
                                m_rangeSelectStartEnd.first = Biggest;
                            }
                            else
                            {
                                m_rangeSelectStartEnd.first = Smallest;
                            }

                        }
                        else
                        {
                            m_rangeSelectStartEnd.first = indexInAlive;
                        }
                        if (m_rangeSelectStartEnd.second == INVALID_ID)
                        {
                            m_rangeSelectStartEnd.second = indexInAlive;
                        }
                    }
                    else
                    {
                        m_rangeSelectStartEnd.second = indexInAlive;
                    }

                }
                else if (ImGui::GetIO().KeyCtrl) //si on appuie sur ctrl en cliquant gauche
                {
                    if (IsSelectedIndex(indexInAlive)) //si l'entitee est deja selectionee on la retire
                    {
                        RemoveEntity(indexInAlive);
                    }
                    else 
                    {
                        AddSelectedEntity(indexInAlive); // si non on l'ajoute aux indices selectionees
                    }
                }
                else
                {
                    SetSelectedEntity(indexInAlive); //si on fait juste clique gauche sur une entitee on la set en selctionee
                }

            }
        }
        //DragNDrop
        if (m_scene.GetComponentStorage<timeline::MeshInstance>().Has(ID))
        {
            m_imGuiWindows.GetDragNDrop()->DropItem<FileEntry, Mesh_ID>(m_scene.GetComponentStorage<timeline::MeshInstance>().Get(ID).meshID);
        }
        else
        {
            Mesh_ID draggedMeshID = INVALID_ID;
            m_imGuiWindows.GetDragNDrop()->DropItem<FileEntry, Mesh_ID>(draggedMeshID);
            if (draggedMeshID != INVALID_ID)
            {
                m_scene.GetComponentStorage<timeline::MeshInstance>().Add(ID, timeline::MeshInstance{ .meshID = draggedMeshID });
            }
        }
        
    }
}

void WindowHierarchy::AddSelectedEntity(size_t _ID)
{
    std::cout << "Add Entity: " << _ID << std::endl;
    if (m_entitiesSelected.contains(_ID))
    {
        m_entitiesSelected.at(_ID) = true;
    }
    else
    {
        m_entitiesSelected.insert({ _ID, true });
    }

    UpdateManagerSelectedItem(_ID);
}

void WindowHierarchy::RemoveEntity(size_t _ID)
{
    if (m_entitiesSelected.contains(_ID))
    {
        m_entitiesSelected.erase(_ID);
    }

    UpdateManagerSelectedItem(_ID);
}

void WindowHierarchy::SetSelectedEntity(size_t _ID)
{
    m_entitiesSelected.clear();
    AddSelectedEntity(_ID);
}

void WindowHierarchy::SetSelectedEntityInRange(size_t _IDStart, size_t _IDEnd)
{
    if (_IDStart > _IDEnd)
    {
        std::swap(_IDStart, _IDEnd);
    }

    for (size_t ID = _IDStart; ID <= _IDEnd ; ID++)
    {
        AddSelectedEntity(ID);
    }
}

bool WindowHierarchy::IsSelectedIndex(size_t index)
{
    if (m_entitiesSelected.contains(index))
    {
        return m_entitiesSelected.at(index);
    }
    return false;
}

size_t WindowHierarchy::GetSmallestSelectedEntity()
{
    if (m_entitiesSelected.size() == 0)
        return INVALID_ENTITY;
    if (m_entitiesSelected.size() == 1)
        return m_entitiesSelected.begin()->first;

    size_t ID = INVALID_ENTITY;
    for (auto& var : m_entitiesSelected)
    {
        ID = var.first < ID ? var.first : ID;
    }
    return ID;
}

size_t WindowHierarchy::GetBiggestSelectedEntity()
{
    if (m_entitiesSelected.size() == 0)
        return INVALID_ENTITY;
    if (m_entitiesSelected.size() == 1)
        return m_entitiesSelected.begin()->first;

    int ID = -1;
    for (auto& var : m_entitiesSelected)
    {
        ID = int(var.first) > ID ? int(var.first) : ID;
    }
    return ID == -1 ? INVALID_ID : ID;
}

std::vector<EntityID> WindowHierarchy::ConstructSelectedEntitiesVector()
{
    std::vector<EntityID> toReturn;
    for (auto& var : m_entitiesSelected)
    {
        if (var.second)
        {
            if (var.first < m_scene.GetAliveEntities().size())
            {
                toReturn.push_back(m_scene.GetAliveEntities()[var.first]);
            }
        }
    }
    return toReturn;
}

void WindowHierarchy::UpdateManagerSelectedItem(size_t _selectedIndex)
{
    if (m_entitiesSelected.size() == 1)
    {
        m_imGuiWindows.SetSelectedItem<EntityID>(m_scene.GetAliveEntities()[_selectedIndex]);
    }
    else
    {
        m_imGuiWindows.SetSelectedItem<std::monostate>({});
    }
}