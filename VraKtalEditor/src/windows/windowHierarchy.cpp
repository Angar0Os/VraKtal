#include "./windows/windowHierarchy.h"
#include "../../include/imGuiWindows.h"
#include "../../include/windows/others/imGuizmoHelper.h"

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

WindowHierarchy::WindowHierarchy(Scene& _scene, ImGuiWindows& _imGuiWindows) : m_scene(_scene), m_imGuiWindows(_imGuiWindows)
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
        m_imGuiWindows.EndWindow("Hierarchy");
    }
}

void WindowHierarchy::DrawEntityHierarchyItem(EntityID ID)
{
    if (!m_scene.GetComponentStorage<std::string>().Has(ID))
        return;

    std::string& label = m_scene.GetEntityComponent<std::string>(ID);

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
        else if (m_imGuiWindows.selectedItem != m_editingEntity)
        {
            m_editingEntity = -1;
        }
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.4f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.5f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.1f, 0.3f, 0.9f, 1.0f));

        if (ImGui::Selectable(label.c_str(), m_imGuiWindows.selectedItem == ID))
        {
            if (m_imGuiWindows.selectedItem != ID)
                m_imGuiWindows.selectedItem = ID;
        }

        ImGui::PopStyleColor(3);

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            m_editingEntity = ID;
            strncpy_s(m_entityRenameBuffer, sizeof(m_entityRenameBuffer), label.c_str(), _TRUNCATE);
        }
    }



}