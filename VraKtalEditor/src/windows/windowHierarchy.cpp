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

WindowHierarchy::WindowHierarchy(Scene& _scene, ImGuiWindows& _imGuiWindows) : m_scene(_scene), m_imGuiWindows(_imGuiWindows) , m_editingEntity(INVALID_ENTITY)
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
        for (EntityID ID : m_scene.GetAliveEntities())
        {
            DrawEntityHierarchyItem(ID);
        }

        if (m_entityRightClicked == INVALID_ENTITY)
        {
            m_imGuiWindows.GetRightClick()->Draw<WindowHierarchy>(this);
        }
        else
        {
            if (!m_imGuiWindows.GetRightClick()->Draw<EntityID>(&m_entityRightClicked))
            {
                m_entityRightClicked = INVALID_ENTITY;
            }
        }


    }
    m_imGuiWindows.EndWindow("Hierarchy");
}

void WindowHierarchy::DrawEntityHierarchyItem(EntityID ID)
{
    if (!m_scene.GetComponentStorage<std::string>().Has(ID))
        return;

    std::string& label = m_scene.GetEntityComponent<std::string>(ID);

    EntityID SelectedItemID = m_imGuiWindows.IsSelectedItemType<EntityID>() ? m_imGuiWindows.GetSelectedItem<EntityID>() : INVALID_ENTITY;
    
        if (m_editingEntity == ID)
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
                m_editingEntity = -1;
            }
            else if (enterPressed)
            {
                label = m_entityRenameBuffer;
                m_editingEntity = -1;
            }
            else if (ImGui::IsItemDeactivatedAfterEdit())
            {
                label = m_entityRenameBuffer;
                m_editingEntity = -1;
            }
            else if (SelectedItemID != m_editingEntity)
            {
                m_editingEntity = -1;
            }
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.4f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.5f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.1f, 0.3f, 0.9f, 1.0f));

            if (ImGui::Selectable(label.c_str(), SelectedItemID == ID))
            {
                if (SelectedItemID != ID)
                {
                    m_imGuiWindows.SetSelectedItem(ID);
                }
            }

            ImGui::PopStyleColor(3);




            if (ImGui::IsItemHovered())
            {
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                {
                    m_entityRightClicked = ID;
                }
                else if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    m_entityRightClicked = INVALID_ENTITY;
                    m_editingEntity = ID;
                    strncpy_s(m_entityRenameBuffer, sizeof(m_entityRenameBuffer), label.c_str(), _TRUNCATE);
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