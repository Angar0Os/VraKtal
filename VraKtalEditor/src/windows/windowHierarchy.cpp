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
#include <scene/timeline/components/light.h>
#include <graphics/renderer.h>
#include <core/gpu/buffer.h>
#include <string>
#include <typeindex>
#include <utility>
#include <iostream>



WindowHierarchy::WindowHierarchy(Scene& _scene, ImGuiWindows& _imGuiWindows) : m_scene(_scene), m_imGuiWindows(_imGuiWindows), m_renamingEntity(INVALID_ENTITY), m_rangeSelectStartEnd({INVALID_ENTITY , INVALID_ENTITY})
{
    m_scene.AddOnEntityCreatedCallBack<WindowHierarchy, &WindowHierarchy::OnEntityCreatedCallBack>(this);
    m_scene.AddOnEntityDestroyedCallBack<WindowHierarchy, &WindowHierarchy::OnEntityDestroyedCallBack>(this);

    AddTypeFilter(typeid(timeline::MeshInstance) , "Mesh");
    AddTypeFilter(typeid(timeline::Light), "Light");

}

WindowHierarchy::~WindowHierarchy()
{
}

void WindowHierarchy::Draw()
{
    ComponentStorage<timeline::MeshInstance>& meshStorage = m_scene.GetComponentStorage<timeline::MeshInstance>();
    if (m_imGuiWindows.BeginWindow("Hierarchy", true))
    {
        DrawFilterBar();
        ImGui::Separator();
        HandleRangeSelect();
        DrawFolders(m_folderManager.m_rootFolder);

        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) &&
            (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) &&
            !ImGui::IsAnyItemHovered())
        {
            std::cout << "Cleared Entity" << std::endl;
            m_entitiesSelected.clear();
        }

        HandleRightClick();

    }
    m_imGuiWindows.EndWindow("Hierarchy");
}

void WindowHierarchy::DrawEntityHierarchyItem(EntityID _ID)
{
    std::string label = m_scene.GetComponentStorage<std::string>().Has(_ID) ? m_scene.GetEntityComponent<std::string>(_ID) : "This Entity Have no name this isn't normal behaviour";

    EntityID SelectedItemID = m_imGuiWindows.IsSelectedItemType<EntityID>() ? m_imGuiWindows.GetSelectedItem<EntityID>() : INVALID_ENTITY;
    bool isEntitySelected = (m_entitiesSelected.contains(_ID) && m_entitiesSelected.at(_ID)) || m_rangeSelectStartEnd.first == _ID ? true : false;

    if (m_renamingEntity == _ID)
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
                if (!m_entitiesSelected.contains(_ID) || !m_entitiesSelected.at(_ID))
                {
                    SetSelectedEntity(_ID);
                }
            }
            else if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                m_renamingEntity = _ID;
                strncpy_s(m_entityRenameBuffer, sizeof(m_entityRenameBuffer), label.c_str(), _TRUNCATE);
            }
            else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                if (ImGui::GetIO().KeyShift)
                {
                    if (m_selectionAnchor != INVALID_ENTITY)
                    {
                        m_rangeSelectStartEnd.first = m_selectionAnchor;
                        m_rangeSelectStartEnd.second = _ID;
                    }
                    else
                    {
                        SetSelectedEntity(_ID);
                        m_selectionAnchor = _ID;
                    }
                }
                else if (ImGui::GetIO().KeyCtrl)
                {
                    if (isEntitySelected) {
                        RemoveEntity(_ID);
                    }
                    else {
                        AddSelectedEntity(_ID);
                    }
                    m_selectionAnchor = _ID;
                }
                else
                {
                    SetSelectedEntity(_ID);
                    m_selectionAnchor = _ID;
                }
            }
        }
        //DragNDrop
        if (m_scene.GetComponentStorage<timeline::MeshInstance>().Has(_ID))
        {
            m_imGuiWindows.GetDragNDrop()->DropItem<FileEntry, Mesh_ID>(m_scene.GetComponentStorage<timeline::MeshInstance>().Get(_ID).meshID);
        }
        else
        {
            Mesh_ID draggedMeshID = INVALID_ID;
            m_imGuiWindows.GetDragNDrop()->DropItem<FileEntry, Mesh_ID>(draggedMeshID);
            if (draggedMeshID != INVALID_ID)
            {
                m_scene.GetComponentStorage<timeline::MeshInstance>().Add(_ID, timeline::MeshInstance{ .meshID = draggedMeshID });
            }
        }
        
    }
}

void WindowHierarchy::HandleRangeSelect()
{
    if (m_rangeSelectStartEnd.first != INVALID_ENTITY && m_rangeSelectStartEnd.second != INVALID_ENTITY)
    {
        m_entitiesSelected.clear();
        for (EntityID entity : m_folderManager.GetEntitiesInRange(m_rangeSelectStartEnd.first, m_rangeSelectStartEnd.second))
        {
            AddSelectedEntity(entity);
        }
        m_rangeSelectStartEnd.first = INVALID_ENTITY;
        m_rangeSelectStartEnd.second = INVALID_ENTITY;
    }
}

void WindowHierarchy::HandleRightClick()
{
    if (m_entitiesSelected.size() == 0)
    {
        m_imGuiWindows.GetRightClick()->Draw<WindowHierarchy>(this);
    }
    else if (m_entitiesSelected.size() == 1)
    {
        EntityID id = m_entitiesSelected.begin()->first;
        m_imGuiWindows.GetRightClick()->Draw<EntityID>(&id);
    }
    else
    {
        std::vector<EntityID> selectedEntities = ConstructSelectedEntitiesVector();
        m_imGuiWindows.GetRightClick()->Draw<std::vector<EntityID>>(&selectedEntities);
    }
}

void WindowHierarchy::OnEntityCreatedCallBack(std::pair<EntityID, size_t> _pair)
{
    m_folderManager.MoveEntityToFolder(_pair.first, m_folderManager.m_rootFolder);
}

void WindowHierarchy::OnEntityDestroyedCallBack(std::pair<EntityID, size_t> _pair)
{
    m_folderManager.RemoveEntityFromAllFolders(_pair.first);
}

void WindowHierarchy::CreateFolder(std::string _name)
{
    hierarchy::FolderID newFolder = m_folderManager.CreateFolder(_name, m_folderManager.m_rootFolder);
    m_renamingFolder = newFolder;
    strncpy_s(m_folderRenameBuffer, sizeof(m_folderRenameBuffer), "New Folder", _TRUNCATE);
}

void WindowHierarchy::AddSelectedEntity(EntityID _ID)
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

void WindowHierarchy::RemoveEntity(EntityID _ID)
{
    if (m_entitiesSelected.contains(_ID))
    {
        m_entitiesSelected.erase(_ID);
    }
    UpdateManagerSelectedItem(_ID);
}

void WindowHierarchy::SetSelectedEntity(EntityID _ID)
{
    m_entitiesSelected.clear();
    AddSelectedEntity(_ID);
}

bool WindowHierarchy::IsSelectedIndex(size_t index)
{
    if (m_entitiesSelected.contains(index))
    {
        return m_entitiesSelected.at(index);
    }
    return false;
}

std::vector<EntityID> WindowHierarchy::ConstructSelectedEntitiesVector()
{
    std::vector<EntityID> toReturn;
    for (auto& var : m_entitiesSelected)
    {
        if (var.second)
        {
           toReturn.push_back(var.first);
        }
    }
    return toReturn;
}

void WindowHierarchy::DrawFilterBar()
{
    float spacing = ImGui::GetStyle().ItemSpacing.x;

    float resetButtonWidth = ImGui::CalcTextSize("Reset").x + ImGui::GetStyle().FramePadding.x * 2.0f;

    float filtersWidth = ImGui::GetContentRegionAvail().x - resetButtonWidth - spacing;

    if (ImGui::Selectable("Filters", false, 0, ImVec2(filtersWidth, 0.0f)))
    {
        ImGui::OpenPopup("HierarchyFiltersPopup");
    }

    ImGui::SameLine();

    if (ImGui::Button("Reset", ImVec2(resetButtonWidth, 0.0f)))
    {
        for (auto& filter : m_typeFilters)
        {
            filter.enabled = false;
        }
    }

    if (ImGui::BeginPopup("HierarchyFiltersPopup"))
    {
        ImGui::Text("Type filters");
        ImGui::Separator();

        for (auto& filter : m_typeFilters)
        {
            ImGui::PushID(filter.name.c_str());

            if (ImGui::Selectable(filter.name.c_str(), filter.enabled))
            {
                filter.enabled = !filter.enabled;
            }

            ImGui::PopID();
        }

        ImGui::EndPopup();
    }
}

void WindowHierarchy::UpdateManagerSelectedItem(EntityID _selectedIndex)
{
    if (m_entitiesSelected.size() == 1)
    {
        m_imGuiWindows.SetSelectedItem<EntityID>(_selectedIndex);
    }
    else
    {
        m_imGuiWindows.SetSelectedItem<std::monostate>({});
    }
}

void WindowHierarchy::AddTypeFilter(std::type_index type, const std::string& name)
{
    for (const TypeFilter& filter : m_typeFilters)
    {
        if (filter.type == type)
            return;
    }

    m_typeFilters.push_back({
        type,
        name,
        false
        });
}

bool WindowHierarchy::PassTypeFilters(EntityID _ID)
{
    bool hasActiveFilter = false;
    for (const TypeFilter& filter : m_typeFilters)
    {
        if (!filter.enabled) //On skip le filter si il est desactivee
            continue;

        hasActiveFilter = true;

        if (filter.type == std::type_index(typeid(timeline::MeshInstance)) &&
            m_scene.GetComponentStorage<timeline::MeshInstance>().Has(_ID))
        {
            return true;
        }

        if (filter.type == std::type_index(typeid(timeline::Light)) &&
            m_scene.GetComponentStorage<timeline::Light>().Has(_ID))
        {
            return true;
        }
    }

    return !hasActiveFilter;
}

void WindowHierarchy::DrawFolders(hierarchy::FolderID _folderID)
{
    hierarchy::Folder& folder = m_folderManager.GetFolder(_folderID);
    const bool isRoot = folder.id == m_folderManager.m_rootFolder;
    if (isRoot)
    {
        // Pas de TreeNode, pas de nom, pas de drag/drop, pas de fermeture.
        for (hierarchy::FolderID childID : folder.children)
        {
            DrawFolders(childID);
        }

        for (EntityID entity : folder.entities)
        {
            DrawEntityHierarchyItem(entity);
        }

        return;
    }
    
    if (m_renamingFolder == folder.id)
    {
        ImGui::SetKeyboardFocusHere();

        bool enterPressed = ImGui::InputText(
            "##RenameFolder",
            m_folderRenameBuffer,
            sizeof(m_folderRenameBuffer),
            ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll
        );
        if (enterPressed || ImGui::IsItemDeactivatedAfterEdit() || ((ImGui::GetMouseClickedCount(ImGuiMouseButton_Left) || ImGui::GetMouseClickedCount(ImGuiMouseButton_Right)) && !ImGui::IsItemClicked()))
        {
            folder.name = m_folderRenameBuffer;
            m_renamingFolder = hierarchy::INVALID_FOLDER;
        }
        return;
    }

    ImGuiTreeNodeFlags isLeaf = (folder.entities.size() > 0 || folder.children.size() > 0) ? ImGuiTreeNodeFlags_None : ImGuiTreeNodeFlags_Leaf;
    ImGuiTreeNodeFlags flags = isLeaf | ImGuiTreeNodeFlags_SpanAvailWidth;

    ImGui::PushID(static_cast<int>(folder.id));
    bool opened = ImGui::TreeNodeEx(folder.name.c_str(), flags);

    if (opened)
    {
        for (hierarchy::FolderID childID : folder.children)
        {
            DrawFolders(childID);
        }

        for (EntityID entity : folder.entities)
        {
            if (PassTypeFilters(entity))
            {
                DrawEntityHierarchyItem(entity);
            }
        }

        ImGui::TreePop();
    }
    ImGui::PopID();
}

