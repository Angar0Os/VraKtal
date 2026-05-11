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
#include <utils/denseStorage.h>



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
    DenseStorage<EntityID,timeline::MeshInstance>& meshStorage = m_scene.GetComponentStorage<timeline::MeshInstance>();
    if (m_imGuiWindows.BeginWindow("Hierarchy", true))
    {
        HandleInputs();

        if (m_pendingDeleteFolder == m_selectedFolder)
        {
            m_selectedFolder = INVALID_ID;
            m_pendingDeleteFolder = INVALID_ID;
        }

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
            m_selectedFolder = INVALID_ENTITY;
            LastTypeSelectedWasFolderId = false;
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
        if (enterPressed || ImGui::IsItemDeactivatedAfterEdit() || ((ImGui::GetMouseClickedCount(ImGuiMouseButton_Left) || ImGui::GetMouseClickedCount(ImGuiMouseButton_Right)) && !ImGui::IsItemClicked()))
        {
            if (m_scene.GetComponentStorage<std::string>().Has(_ID))
            {
                m_scene.GetComponentStorage<std::string>().Get(_ID) = m_entityRenameBuffer;
                m_renamingEntity = INVALID_ENTITY;
            }
            else
            {

                m_scene.GetComponentStorage<std::string>().Add(_ID, m_entityRenameBuffer);
            }
        }
    }
    else
    {

        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.4f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.5f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.1f, 0.3f, 0.9f, 1.0f));
    
        ImGui::Selectable(label.c_str(), isEntitySelected);
        ImGui::PopStyleColor(3);

        EntityPayload payload;
        if (isEntitySelected)
        {
            for (auto& [entity, selected] : m_entitiesSelected)
            {
                if (selected && payload.count < 128)
                {
                    payload.entities[payload.count++] = entity;
                }
            }
        }
        else
        {
            payload.entities[payload.count++] = _ID;
        }

        m_imGuiWindows.GetDragNDrop()->Drag<EntityPayload>(payload);

        if (ImGui::IsItemHovered())
        {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            {
                if (!isEntitySelected)
                {
                    SetSelectedEntity(_ID);
                    m_selectionAnchor = _ID;
                }
            }
            else if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                m_renamingEntity = _ID;
                strncpy_s(m_entityRenameBuffer, sizeof(m_entityRenameBuffer), label.c_str(), _TRUNCATE);
            }
            else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !ImGui::IsMouseDragging(ImGuiMouseButton_Left))
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
                    if (isEntitySelected)
                        RemoveEntityFromSelected(_ID);
                    else
                        AddSelectedEntity(_ID);

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
            m_imGuiWindows.GetDragNDrop()->DropItem<FileEntry, Mesh_ID>(m_scene.GetComponentStorage<timeline::MeshInstance>().Get(_ID).assetID);
        }
        else
        {
            Mesh_ID draggedMeshID = INVALID_ID;
            m_imGuiWindows.GetDragNDrop()->DropItem<FileEntry, Mesh_ID>(draggedMeshID);
            if (draggedMeshID != INVALID_ID)
            {
                m_scene.GetComponentStorage<timeline::MeshInstance>().Add(_ID, timeline::MeshInstance{ .assetID = draggedMeshID });
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
    if (LastTypeSelectedWasFolderId && m_selectedFolder != INVALID_ENTITY)
    {
        m_imGuiWindows.GetRightClick()->Draw<WindowHierarchy , hierarchy::Folder>(this , &m_folderManager.GetFolder(m_selectedFolder));
    }
    else if (m_entitiesSelected.size() > 0)
    {
        if (m_entitiesSelected.size() == 1)
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
    else
    {
        m_imGuiWindows.GetRightClick()->Draw<WindowHierarchy>(this);
    }
}

void WindowHierarchy::HandleInputs()
{
    if (!ImGui::IsWindowFocused())
        return;

    /*
        On pourrait utiliser notre system d'input pour ca.
        Je sais que c'est pas quelque chose qu'imGui recommande donc je sais pas si c'est une bonne idee
    */

    if (LastTypeSelectedWasFolderId && m_selectedFolder != INVALID_ID)
    {
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) || ImGui::IsKeyPressed(ImGuiKey_F2))
        {
            RenameFolder(m_selectedFolder);
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_Delete))
        {
            if (ImGui::IsKeyPressed(ImGuiKey_LeftShift))
            {
                DeleteFolderAndContent(m_selectedFolder);
            }
            else
            {
                DeleteFolder(m_selectedFolder);
            }

        }
        
    }
    else if (!LastTypeSelectedWasFolderId && m_entitiesSelected.size() > 0)
    {
        if (ImGui::IsKeyPressed(ImGuiKey_Delete))
        {
            for (auto& var : m_entitiesSelected)
            {
                m_scene.DestroyEntity(var.first);
            }
        }
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

    LastTypeSelectedWasFolderId = false;

    UpdateManagerSelectedItem(_ID);
}

void WindowHierarchy::RemoveEntityFromSelected(EntityID _ID)
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

bool WindowHierarchy::IsEntitySelected(EntityID index)
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
    if (LastTypeSelectedWasFolderId)
    {
        m_entitiesSelected.clear();
        m_imGuiWindows.SetSelectedItem<hierarchy::Folder*>(&m_folderManager.GetFolder(m_selectedFolder));
    }
    else
    {
        m_selectedFolder = 0;
        if (m_entitiesSelected.size() == 1)
        {
            m_imGuiWindows.SetSelectedItem<EntityID>(_selectedIndex);
        }
        else
        {
            m_imGuiWindows.SetSelectedItem<std::monostate>({});
        }
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
        //Handle Dropped Folder on Folder 
        hierarchy::FolderID droppedFolderID = INVALID_ENTITY;
        hierarchy::Folder* Droppedfolder = m_imGuiWindows.GetDragNDrop()->DropWindow<hierarchy::Folder, hierarchy::FolderID>(droppedFolderID);
        if (Droppedfolder != nullptr)
        {
            m_folderManager.MoveFolderToFolder(Droppedfolder->id, folder.id);
        }
        EntityPayload* payload = m_imGuiWindows.GetDragNDrop()->DropItem<EntityPayload>();
        if (payload != nullptr)
        {
            for (uint32_t i = 0; i < payload->count; i++)
            {
                m_folderManager.MoveEntityToFolder(payload->entities[i], folder.id);
            }
        }

        // Pas de TreeNode, pas de nom, pas de fermeture pour le root il est puni
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
    bool bIsSelected = m_selectedFolder == folder.id;
    ImGuiTreeNodeFlags flags =
        isLeaf | //pour la fleche
        ImGuiTreeNodeFlags_SpanAvailWidth | //pour la largeur
        (bIsSelected ? ImGuiTreeNodeFlags_Selected : 0) | //pour la couleur
        (folder.open ? ImGuiTreeNodeFlags_DefaultOpen : 0) //pour ouvrir le node f(est-ce que le folder est ouvert)
        ;

    ImGui::PushID(static_cast<int>(folder.id));
    bool opened = ImGui::TreeNodeEx(folder.name.c_str(), flags);
    if (ImGui::IsItemHovered()) // Rename
    {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            SelectFolder(folder.id);
        }
    }

    //Handle Drag of Folder
    m_imGuiWindows.GetDragNDrop()->Drag<hierarchy::Folder>(folder);
    //Handle Dropped Folder on Folder 
    hierarchy::FolderID droppedFolderID = INVALID_ENTITY;
    hierarchy::Folder* Droppedfolder = m_imGuiWindows.GetDragNDrop()->DropItem<hierarchy::Folder, hierarchy::FolderID>(droppedFolderID);
    if (Droppedfolder != nullptr)
    {
        m_folderManager.MoveFolderToFolder(Droppedfolder->id, folder.id);
    }

    EntityPayload* payload = m_imGuiWindows.GetDragNDrop()->DropItem<EntityPayload>();
    if (payload != nullptr)
    {
        for (uint32_t i = 0; i < payload->count; i++)
        {
            m_folderManager.MoveEntityToFolder(payload->entities[i], folder.id);
        }
    }
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

void WindowHierarchy::RenameFolder(hierarchy::FolderID _folderID)
{
    m_renamingFolder = _folderID;
    strncpy_s(m_folderRenameBuffer, sizeof(m_folderRenameBuffer), "New Folder", _TRUNCATE);
}

void WindowHierarchy::DeleteFolder(hierarchy::FolderID _folderID)
{
    m_pendingDeleteFolder = _folderID;
    m_folderManager.DeleteFolder(_folderID);
}

void WindowHierarchy::DeleteFolderAndContent(hierarchy::FolderID _folderID)
{

    for (hierarchy::FolderID childFolder : m_folderManager.GetFolder(_folderID).children)
    {
        m_folderManager.DeleteFolder(childFolder);
    }
    //on doit faire une copie a cause du bind OnDestroyedEntity
    std::vector<EntityID> entities = m_folderManager.GetFolder(_folderID).entities;

    for (EntityID entity : entities)
    {
        m_scene.DestroyEntity(entity);
    }

    DeleteFolder(_folderID);

}

void WindowHierarchy::SelectFolder(hierarchy::FolderID _folderID)
{
    m_selectedFolder = _folderID;
    LastTypeSelectedWasFolderId = true;
    UpdateManagerSelectedItem(INVALID_ENTITY);
}