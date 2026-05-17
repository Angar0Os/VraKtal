#include "./windows/windowHierarchy.h"
#include "../../include/imGuiWindows.h"

#include "../../include/windows/others/imGuizmoHelper.h"
#include "../../include/windows/others/dragNdrop.h"

#include <factory/meshFactory.h>

#include "../../include/windows/popup/rightClick.h"

#include <imgui/imgui.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <core/manager/sceneManager.h>
#include <scene/scene.h>
#include <scene/timeline/components/mesh.h>
#include <scene/timeline/components/light.h>
#include <graphics/renderer.h>
#include <core/gpu/buffer.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>
#include <typeindex>
#include <utility>
#include <variant>
#include <vector>

#include <utils/denseStorage.h>

namespace
{
    bool IsEntityAlive(Scene& _scene, EntityID _entity)
    {
        std::vector<EntityID>& aliveEntities = _scene.GetAliveEntities();
        return std::find(aliveEntities.begin(), aliveEntities.end(), _entity) != aliveEntities.end();
    }

    void AddUniqueEntity(std::vector<EntityID>& _entities, EntityID _entity)
    {
        if (std::find(_entities.begin(), _entities.end(), _entity) == _entities.end())
            _entities.push_back(_entity);
    }

    void CollectEntitiesInFolderTree(
        hierarchy::FolderManager& _folderManager,
        hierarchy::FolderID _folderID,
        std::vector<EntityID>& _entities)
    {
        hierarchy::Folder& folder = _folderManager.GetFolder(_folderID);

        for (EntityID entity : folder.entities)
            AddUniqueEntity(_entities, entity);

        const std::vector<hierarchy::FolderID> children = folder.children;

        for (hierarchy::FolderID childID : children)
            CollectEntitiesInFolderTree(_folderManager, childID, _entities);
    }

    void CollectEntityFolderOccurrences(
        hierarchy::FolderManager& _folderManager,
        hierarchy::FolderID _folderID,
        EntityID _entity,
        std::vector<hierarchy::FolderID>& _folders)
    {
        hierarchy::Folder& folder = _folderManager.GetFolder(_folderID);

        if (std::find(folder.entities.begin(), folder.entities.end(), _entity) != folder.entities.end())
            _folders.push_back(_folderID);

        const std::vector<hierarchy::FolderID> children = folder.children;

        for (hierarchy::FolderID childID : children)
            CollectEntityFolderOccurrences(_folderManager, childID, _entity, _folders);
    }

    bool FolderContainsEntity(
        hierarchy::FolderManager& _folderManager,
        hierarchy::FolderID _folderID,
        EntityID _entity)
    {
        hierarchy::Folder& folder = _folderManager.GetFolder(_folderID);

        if (std::find(folder.entities.begin(), folder.entities.end(), _entity) != folder.entities.end())
            return true;

        const std::vector<hierarchy::FolderID> children = folder.children;

        for (hierarchy::FolderID childID : children)
        {
            if (FolderContainsEntity(_folderManager, childID, _entity))
                return true;
        }

        return false;
    }

    bool IsFolderDescendant(
        hierarchy::FolderManager& _folderManager,
        hierarchy::FolderID _ancestor,
        hierarchy::FolderID _possibleDescendant)
    {
        if (_ancestor == _possibleDescendant)
            return true;

        hierarchy::Folder& ancestor = _folderManager.GetFolder(_ancestor);

        const std::vector<hierarchy::FolderID> children = ancestor.children;

        for (hierarchy::FolderID childID : children)
        {
            if (IsFolderDescendant(_folderManager, childID, _possibleDescendant))
                return true;
        }

        return false;
    }

    bool CanMoveFolderToFolder(
        hierarchy::FolderManager& _folderManager,
        hierarchy::FolderID _movedFolder,
        hierarchy::FolderID _targetFolder)
    {
        if (_movedFolder == hierarchy::INVALID_FOLDER)
            return false;

        if (_targetFolder == hierarchy::INVALID_FOLDER)
            return false;

        if (_movedFolder == _targetFolder)
            return false;

        if (_movedFolder == _folderManager.m_rootFolder)
            return false;

        // Empêche de déplacer un parent dans un de ses propres enfants.
        if (IsFolderDescendant(_folderManager, _movedFolder, _targetFolder))
            return false;

        return true;
    }

    void CollectFolderSubtree(
        hierarchy::FolderManager& _folderManager,
        hierarchy::FolderID _folderID,
        std::vector<hierarchy::FolderID>& _folders,
        std::vector<EntityID>& _entities)
    {
        hierarchy::Folder& folder = _folderManager.GetFolder(_folderID);

        for (EntityID entity : folder.entities)
            AddUniqueEntity(_entities, entity);

        const std::vector<hierarchy::FolderID> children = folder.children;

        for (hierarchy::FolderID childID : children)
            CollectFolderSubtree(_folderManager, childID, _folders, _entities);

        _folders.push_back(_folderID);
    }
}

WindowHierarchy::WindowHierarchy(SceneManager& _sceneManager, ImGuiWindows& _imGuiWindows)
    : m_sceneManager(_sceneManager)
    , m_imGuiWindows(_imGuiWindows)
    , m_currentSceneID(INVALID_ENTITY)
    , m_renamingEntity(INVALID_ENTITY)
    , m_rangeSelectStartEnd({ INVALID_ENTITY, INVALID_ENTITY })
{
    AddTypeFilter(typeid(timeline::MeshInstance), "Mesh");
    AddTypeFilter(typeid(timeline::Light), "Light");

    SyncActiveScene();
}

WindowHierarchy::~WindowHierarchy()
{
}

Scene* WindowHierarchy::GetActiveScene()
{
    EntityID sceneID = GetActiveSceneID();

    if (sceneID == INVALID_ENTITY)
        return nullptr;

    return &m_sceneManager.GetScene(sceneID);
}

const Scene* WindowHierarchy::GetActiveScene() const
{
    EntityID sceneID = GetActiveSceneID();

    if (sceneID == INVALID_ENTITY)
        return nullptr;

    return &m_sceneManager.GetScene(sceneID);
}

EntityID WindowHierarchy::GetActiveSceneID() const
{
    if (m_currentSceneID != INVALID_ENTITY &&
        m_sceneManager.IsValidScene(m_currentSceneID) &&
        m_sceneManager.IsSceneActive(m_currentSceneID))
    {
        return m_currentSceneID;
    }

    const std::vector<EntityID>& activeSceneIDs = m_sceneManager.GetActiveSceneIDs();

    for (EntityID sceneID : activeSceneIDs)
    {
        if (m_sceneManager.IsValidScene(sceneID))
            return sceneID;
    }

    return INVALID_ENTITY;
}

hierarchy::FolderManager* WindowHierarchy::GetFolderManager()
{
    if (GetActiveSceneID() == INVALID_ENTITY)
        return nullptr;

    return &GetActiveFolderManager();
}

hierarchy::FolderManager& WindowHierarchy::GetActiveFolderManager()
{
    EntityID sceneID = GetActiveSceneID();

    auto [it, inserted] = m_folderManagers.try_emplace(sceneID);
    return it->second;
}

void WindowHierarchy::BindActiveSceneCallbacks()
{
    // Plus utilisé.
    // Avec un SceneManager, le callback actuel ne donne pas le sceneID.
    // Donc on préfère synchroniser la hiérarchie depuis Scene::GetAliveEntities().
}

void WindowHierarchy::SyncActiveScene()
{
    const EntityID resolvedSceneID = GetActiveSceneID();

    if (m_currentSceneID != resolvedSceneID)
    {
        m_currentSceneID = resolvedSceneID;

        ClearSelection();

        m_renamingEntity = INVALID_ENTITY;
        m_renamingFolder = hierarchy::INVALID_FOLDER;
        m_pendingDeleteFolder = hierarchy::INVALID_FOLDER;
    }

    if (m_currentSceneID == INVALID_ENTITY)
        return;

    if (!m_sceneManager.IsValidScene(m_currentSceneID))
        return;

    Scene& scene = m_sceneManager.GetScene(m_currentSceneID);
    hierarchy::FolderManager& folderManager = GetActiveFolderManager();

    SyncFolderManagerWithScene(scene, folderManager);
}

void WindowHierarchy::ClearSelection()
{
    m_entitiesSelected.clear();

    m_selectedFolder = hierarchy::INVALID_FOLDER;
    LastTypeSelectedWasFolderId = false;

    m_selectionAnchor = INVALID_ENTITY;
    m_rangeSelectStartEnd = { INVALID_ENTITY, INVALID_ENTITY };

    m_imGuiWindows.SetSelectedItem<std::monostate>({});
}

void WindowHierarchy::SyncFolderManagerWithScene(Scene& _scene, hierarchy::FolderManager& _folderManager)
{
    std::vector<EntityID> entitiesInHierarchy;
    CollectEntitiesInFolderTree(_folderManager, _folderManager.m_rootFolder, entitiesInHierarchy);

    for (EntityID entity : entitiesInHierarchy)
    {
        if (!IsEntityAlive(_scene, entity))
            _folderManager.RemoveEntityFromAllFolders(entity);
    }

    for (EntityID entity : _scene.GetAliveEntities())
    {
        std::vector<hierarchy::FolderID> occurrences;
        CollectEntityFolderOccurrences(_folderManager, _folderManager.m_rootFolder, entity, occurrences);

        if (occurrences.empty())
        {
            _folderManager.MoveEntityToFolder(entity, _folderManager.m_rootFolder);
        }
        else if (occurrences.size() > 1)
        {
            hierarchy::FolderID folderToKeep = occurrences.front();

            _folderManager.RemoveEntityFromAllFolders(entity);
            _folderManager.MoveEntityToFolder(entity, folderToKeep);
        }
    }
}

void WindowHierarchy::Draw()
{
    SyncActiveScene();

    if (m_imGuiWindows.BeginWindow("Hierarchy", true))
    {
        const std::vector<EntityID>& activeSceneIDs = m_sceneManager.GetActiveSceneIDs();

        if (activeSceneIDs.empty())
        {
            ImGui::TextUnformatted("No active scene.");
            m_imGuiWindows.EndWindow("Hierarchy");
            return;
        }

        EntityID currentSceneID = GetActiveSceneID();

        std::string currentSceneLabel = "No scene";

        if (currentSceneID != INVALID_ENTITY && m_sceneManager.IsValidScene(currentSceneID))
        {
            Scene& currentScene = m_sceneManager.GetScene(currentSceneID);
            currentSceneLabel = currentScene.GetName().empty()
                ? "Scene " + std::to_string(currentSceneID)
                : currentScene.GetName();
            m_imGuiWindows.SetScene(currentSceneID);
        }

        if (ImGui::BeginCombo("Scene", currentSceneLabel.c_str()))
        {
            for (EntityID sceneID : activeSceneIDs)
            {
                if (!m_sceneManager.IsValidScene(sceneID))
                    continue;

                Scene& scene = m_sceneManager.GetScene(sceneID);

                std::string label = scene.GetName().empty()
                    ? "Scene " + std::to_string(sceneID)
                    : scene.GetName();

                const bool selected = sceneID == currentSceneID;

                if (ImGui::Selectable(label.c_str(), selected))
                {
                    if (m_currentSceneID != sceneID)
                    {
                        m_currentSceneID = sceneID;

                        ClearSelection();

                        m_renamingEntity = INVALID_ENTITY;
                        m_renamingFolder = hierarchy::INVALID_FOLDER;
                        m_pendingDeleteFolder = hierarchy::INVALID_FOLDER;

                        if (m_sceneManager.IsValidScene(m_currentSceneID))
                        {
                            Scene& selectedScene = m_sceneManager.GetScene(m_currentSceneID);
                            hierarchy::FolderManager& folderManager = GetActiveFolderManager();
                            SyncFolderManagerWithScene(selectedScene, folderManager);
                        }
                    }
                }

                if (selected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        Scene* scene = GetActiveScene();

        if (scene == nullptr)
        {
            ImGui::TextUnformatted("Selected scene is invalid.");
            m_imGuiWindows.EndWindow("Hierarchy");
            return;
        }

        HandleInputs();

        if (m_pendingDeleteFolder == m_selectedFolder)
        {
            m_selectedFolder = hierarchy::INVALID_FOLDER;
            m_pendingDeleteFolder = hierarchy::INVALID_FOLDER;
        }

        DrawFilterBar();
        ImGui::Separator();

        HandleRangeSelect();

        hierarchy::FolderManager& folderManager = GetActiveFolderManager();
        DrawFolders(folderManager.m_rootFolder);

        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) &&
            (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) &&
            !ImGui::IsAnyItemHovered())
        {
            ClearSelection();
        }

        HandleRightClick();
    }

    m_imGuiWindows.EndWindow("Hierarchy");
}

void WindowHierarchy::DrawEntityHierarchyItem(EntityID _ID)
{
    Scene* scenePtr = GetActiveScene();

    if (scenePtr == nullptr)
        return;

    Scene& scene = *scenePtr;

    ImGui::PushID(static_cast<int>(_ID));

    std::string label = scene.GetComponentStorage<std::string>().Has(_ID)
        ? scene.GetEntityComponent<std::string>(_ID)
        : "Unnamed Entity";

    const bool isEntitySelected =
        (m_entitiesSelected.contains(_ID) && m_entitiesSelected.at(_ID)) ||
        m_rangeSelectStartEnd.first == _ID;

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

        const bool cancelClick =
            (ImGui::GetMouseClickedCount(ImGuiMouseButton_Left) ||
                ImGui::GetMouseClickedCount(ImGuiMouseButton_Right)) &&
            !ImGui::IsItemClicked();

        if (enterPressed || ImGui::IsItemDeactivatedAfterEdit() || cancelClick)
        {
            if (scene.GetComponentStorage<std::string>().Has(_ID))
                scene.GetComponentStorage<std::string>().Get(_ID) = m_entityRenameBuffer;
            else
                scene.GetComponentStorage<std::string>().Add(_ID, std::string(m_entityRenameBuffer));

            m_renamingEntity = INVALID_ENTITY;
        }

        ImGui::PopID();
        return;
    }

    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.4f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.5f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.1f, 0.3f, 0.9f, 1.0f));

    ImGui::Selectable(label.c_str(), isEntitySelected);

    ImGui::PopStyleColor(3);

    EntityPayload payload{};

    if (isEntitySelected)
    {
        for (auto& [entity, selected] : m_entitiesSelected)
        {
            if (selected && payload.count < 128)
                payload.entities[payload.count++] = entity;
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
        else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) &&
            !ImGui::IsMouseDragging(ImGuiMouseButton_Left))
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

    if (scene.GetComponentStorage<timeline::MeshInstance>().Has(_ID))
    {
        m_imGuiWindows.GetDragNDrop()->DropItem<graphics::assets::Mesh, Mesh_ID>(
            scene.GetComponentStorage<timeline::MeshInstance>().Get(_ID).assetID
        );
    }
    else
    {
        Mesh_ID draggedMeshID = INVALID_ID;
        m_imGuiWindows.GetDragNDrop()->DropItem<graphics::assets::Mesh, Mesh_ID>(draggedMeshID);
        if (draggedMeshID != INVALID_ID)
        {
            scene.GetComponentStorage<timeline::MeshInstance>().Add(
                _ID,
                timeline::MeshInstance{ .assetID = draggedMeshID }
            );
        }
    }
    ImGui::PopID();
}

void WindowHierarchy::HandleRangeSelect()
{
    if (m_rangeSelectStartEnd.first == INVALID_ENTITY ||
        m_rangeSelectStartEnd.second == INVALID_ENTITY)
    {
        return;
    }

    hierarchy::FolderManager& folderManager = GetActiveFolderManager();

    m_entitiesSelected.clear();

    for (EntityID entity : folderManager.GetEntitiesInRange(
        m_rangeSelectStartEnd.first,
        m_rangeSelectStartEnd.second))
    {
        AddSelectedEntity(entity);
    }

    m_rangeSelectStartEnd.first = INVALID_ENTITY;
    m_rangeSelectStartEnd.second = INVALID_ENTITY;
}

void WindowHierarchy::HandleRightClick()
{
    hierarchy::FolderManager& folderManager = GetActiveFolderManager();

    if (LastTypeSelectedWasFolderId && m_selectedFolder != hierarchy::INVALID_FOLDER)
    {
        m_imGuiWindows.GetRightClick()->Draw<WindowHierarchy, hierarchy::Folder>(
            this,
            &folderManager.GetFolder(m_selectedFolder)
        );
    }
    else if (!m_entitiesSelected.empty())
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
    Scene* scene = GetActiveScene();

    if (scene == nullptr)
        return;

    if (!ImGui::IsWindowFocused())
        return;

    if (LastTypeSelectedWasFolderId && m_selectedFolder != hierarchy::INVALID_FOLDER)
    {
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) || ImGui::IsKeyPressed(ImGuiKey_F2))
        {
            RenameFolder(m_selectedFolder);
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_Delete))
        {
            if (ImGui::GetIO().KeyShift)
                DeleteFolderAndContent(m_selectedFolder);
            else
                DeleteFolder(m_selectedFolder);
        }
    }
    else if (!LastTypeSelectedWasFolderId && !m_entitiesSelected.empty())
    {
        if (ImGui::IsKeyPressed(ImGuiKey_Delete))
        {
            std::vector<EntityID> entitiesToDelete = ConstructSelectedEntitiesVector();

            for (EntityID entity : entitiesToDelete)
            {
                if (IsEntityAlive(*scene, entity))
                    scene->DestroyEntity(entity);
            }

            ClearSelection();
        }
    }
}

void WindowHierarchy::OnEntityCreatedCallBack(std::pair<EntityID, size_t> _pair)
{
    // Plus utilisé.
}

void WindowHierarchy::OnEntityDestroyedCallBack(std::pair<EntityID, size_t> _pair)
{
    // Plus utilisé.
}

void WindowHierarchy::CreateFolder(std::string _name)
{
    if (GetActiveSceneID() == INVALID_ENTITY)
        return;

    hierarchy::FolderManager& folderManager = GetActiveFolderManager();

    hierarchy::FolderID parentFolder = m_selectedFolder != hierarchy::INVALID_FOLDER
        ? m_selectedFolder
        : folderManager.m_rootFolder;

    hierarchy::FolderID newFolder = folderManager.CreateFolder(_name, parentFolder);

    folderManager.GetFolder(parentFolder).open = true;

    SelectFolder(newFolder);
}

void WindowHierarchy::AddSelectedEntity(EntityID _ID)
{
    m_entitiesSelected[_ID] = true;

    LastTypeSelectedWasFolderId = false;
    m_selectedFolder = hierarchy::INVALID_FOLDER;

    UpdateManagerSelectedItem(_ID);
}

void WindowHierarchy::RemoveEntityFromSelected(EntityID _ID)
{
    m_entitiesSelected.erase(_ID);

    if (m_entitiesSelected.empty())
        m_imGuiWindows.SetSelectedItem<std::monostate>({});
    else
        UpdateManagerSelectedItem(INVALID_ENTITY);
}

void WindowHierarchy::SetSelectedEntity(EntityID _ID)
{
    m_entitiesSelected.clear();
    AddSelectedEntity(_ID);
}

bool WindowHierarchy::IsEntitySelected(EntityID index)
{
    auto it = m_entitiesSelected.find(index);
    return it != m_entitiesSelected.end() && it->second;
}

std::vector<EntityID> WindowHierarchy::ConstructSelectedEntitiesVector()
{
    std::vector<EntityID> selectedEntities;

    for (auto& [entity, selected] : m_entitiesSelected)
    {
        if (selected)
            selectedEntities.push_back(entity);
    }

    return selectedEntities;
}

void WindowHierarchy::DrawFilterBar()
{
    const float spacing = ImGui::GetStyle().ItemSpacing.x;

    const float resetButtonWidth =
        ImGui::CalcTextSize("Reset").x +
        ImGui::GetStyle().FramePadding.x * 2.0f;

    const float filtersWidth =
        ImGui::GetContentRegionAvail().x -
        resetButtonWidth -
        spacing;

    if (ImGui::Selectable("Filters", false, 0, ImVec2(filtersWidth, 0.0f)))
        ImGui::OpenPopup("HierarchyFiltersPopup");

    ImGui::SameLine();

    if (ImGui::Button("Reset", ImVec2(resetButtonWidth, 0.0f)))
    {
        for (auto& filter : m_typeFilters)
            filter.enabled = false;
    }

    if (ImGui::BeginPopup("HierarchyFiltersPopup"))
    {
        ImGui::Text("Type filters");
        ImGui::Separator();

        for (auto& filter : m_typeFilters)
        {
            ImGui::PushID(filter.name.c_str());

            if (ImGui::Selectable(filter.name.c_str(), filter.enabled))
                filter.enabled = !filter.enabled;

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

        if (m_selectedFolder != hierarchy::INVALID_FOLDER)
        {
            hierarchy::FolderManager& folderManager = GetActiveFolderManager();

            m_imGuiWindows.SetSelectedItem<hierarchy::Folder*>(
                &folderManager.GetFolder(m_selectedFolder)
            );
        }
        else
        {
            m_imGuiWindows.SetSelectedItem<std::monostate>({});
        }
    }
    else
    {
        m_selectedFolder = hierarchy::INVALID_FOLDER;

        if (m_entitiesSelected.size() == 1 && _selectedIndex != INVALID_ENTITY)
            m_imGuiWindows.SetSelectedItem<EntityID>(_selectedIndex);
        else
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

    m_typeFilters.push_back(TypeFilter{
        type,
        name,
        false
        });
}

bool WindowHierarchy::PassTypeFilters(EntityID _ID)
{
    Scene* scene = GetActiveScene();

    if (scene == nullptr)
        return false;

    bool hasActiveFilter = false;

    for (const TypeFilter& filter : m_typeFilters)
    {
        if (!filter.enabled)
            continue;

        hasActiveFilter = true;

        if (filter.type == std::type_index(typeid(timeline::MeshInstance)) &&
            scene->GetComponentStorage<timeline::MeshInstance>().Has(_ID))
        {
            return true;
        }

        if (filter.type == std::type_index(typeid(timeline::Light)) &&
            scene->GetComponentStorage<timeline::Light>().Has(_ID))
        {
            return true;
        }
    }

    return !hasActiveFilter;
}

void WindowHierarchy::DrawFolders(hierarchy::FolderID _folderID)
{
    hierarchy::FolderManager& folderManager = GetActiveFolderManager();
    hierarchy::Folder& folder = folderManager.GetFolder(_folderID);

    const bool isRoot = folder.id == folderManager.m_rootFolder;

    if (isRoot)
    {
        hierarchy::FolderID droppedFolderID = hierarchy::INVALID_FOLDER;

        hierarchy::Folder* droppedFolder =
            m_imGuiWindows.GetDragNDrop()->DropWindow<hierarchy::Folder, hierarchy::FolderID>(
                droppedFolderID
            );

        if (droppedFolder != nullptr &&
            CanMoveFolderToFolder(folderManager, droppedFolder->id, folder.id))
        {
            folderManager.MoveFolderToFolder(droppedFolder->id, folder.id);
        }

        EntityPayload* payload = m_imGuiWindows.GetDragNDrop()->DropItem<EntityPayload>();

        if (payload != nullptr)
        {
            for (uint32_t i = 0; i < payload->count; ++i)
                folderManager.MoveEntityToFolder(payload->entities[i], folder.id);
        }

        const std::vector<hierarchy::FolderID> children = folder.children;
        const std::vector<EntityID> entities = folder.entities;

        for (hierarchy::FolderID childID : children)
            DrawFolders(childID);

        for (EntityID entity : entities)
        {
            if (PassTypeFilters(entity))
                DrawEntityHierarchyItem(entity);
        }

        return;
    }

    if (m_renamingFolder == folder.id)
    {
        ImGui::SetKeyboardFocusHere();

        const bool enterPressed = ImGui::InputText(
            "##RenameFolder",
            m_folderRenameBuffer,
            sizeof(m_folderRenameBuffer),
            ImGuiInputTextFlags_EnterReturnsTrue |
            ImGuiInputTextFlags_AutoSelectAll
        );

        const bool cancelClick =
            (ImGui::GetMouseClickedCount(ImGuiMouseButton_Left) ||
                ImGui::GetMouseClickedCount(ImGuiMouseButton_Right)) &&
            !ImGui::IsItemClicked();

        if (enterPressed || ImGui::IsItemDeactivatedAfterEdit() || cancelClick)
        {
            folder.name = m_folderRenameBuffer;
            m_renamingFolder = hierarchy::INVALID_FOLDER;
        }

        return;
    }

    const ImGuiTreeNodeFlags isLeaf =
        (folder.entities.empty() && folder.children.empty())
        ? ImGuiTreeNodeFlags_Leaf
        : ImGuiTreeNodeFlags_None;

    const bool isSelected = m_selectedFolder == folder.id;

    ImGuiTreeNodeFlags flags =
        isLeaf |
        ImGuiTreeNodeFlags_SpanAvailWidth |
        (isSelected ? ImGuiTreeNodeFlags_Selected : 0) |
        (folder.open ? ImGuiTreeNodeFlags_DefaultOpen : 0);

    ImGui::PushID(static_cast<int>(folder.id));

    const bool opened = ImGui::TreeNodeEx(folder.name.c_str(), flags);

    if (ImGui::IsItemHovered())
    {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) ||
            ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            SelectFolder(folder.id);
        }
    }

    m_imGuiWindows.GetDragNDrop()->Drag<hierarchy::Folder>(folder);

    hierarchy::FolderID droppedFolderID = hierarchy::INVALID_FOLDER;

    hierarchy::Folder* droppedFolder =
        m_imGuiWindows.GetDragNDrop()->DropItem<hierarchy::Folder, hierarchy::FolderID>(
            droppedFolderID
        );

    if (droppedFolder != nullptr &&
        CanMoveFolderToFolder(folderManager, droppedFolder->id, folder.id))
    {
        folderManager.MoveFolderToFolder(droppedFolder->id, folder.id);
    }

    EntityPayload* payload = m_imGuiWindows.GetDragNDrop()->DropItem<EntityPayload>();

    if (payload != nullptr)
    {
        for (uint32_t i = 0; i < payload->count; ++i)
            folderManager.MoveEntityToFolder(payload->entities[i], folder.id);
    }

    if (opened)
    {
        const std::vector<hierarchy::FolderID> children = folder.children;
        const std::vector<EntityID> entities = folder.entities;

        for (hierarchy::FolderID childID : children)
            DrawFolders(childID);

        for (EntityID entity : entities)
        {
            if (PassTypeFilters(entity))
                DrawEntityHierarchyItem(entity);
        }

        ImGui::TreePop();
    }

    ImGui::PopID();
}

void WindowHierarchy::RenameFolder(hierarchy::FolderID _folderID)
{
    if (_folderID == hierarchy::INVALID_FOLDER)
        return;

    hierarchy::FolderManager& folderManager = GetActiveFolderManager();

    if (_folderID == folderManager.m_rootFolder)
        return;

    m_renamingFolder = _folderID;

    strncpy_s(
        m_folderRenameBuffer,
        sizeof(m_folderRenameBuffer),
        folderManager.GetFolder(_folderID).name.c_str(),
        _TRUNCATE
    );
}

void WindowHierarchy::DeleteFolder(hierarchy::FolderID _folderID)
{
    if (_folderID == hierarchy::INVALID_FOLDER)
        return;

    hierarchy::FolderManager& folderManager = GetActiveFolderManager();

    if (_folderID == folderManager.m_rootFolder)
        return;

    m_pendingDeleteFolder = _folderID;

    folderManager.DeleteFolder(_folderID);
}

void WindowHierarchy::DeleteFolderAndContent(hierarchy::FolderID _folderID)
{
    if (_folderID == hierarchy::INVALID_FOLDER)
        return;

    Scene* scene = GetActiveScene();

    if (scene == nullptr)
        return;

    hierarchy::FolderManager& folderManager = GetActiveFolderManager();

    if (_folderID == folderManager.m_rootFolder)
        return;

    std::vector<hierarchy::FolderID> foldersToDelete;
    std::vector<EntityID> entitiesToDestroy;

    CollectFolderSubtree(folderManager, _folderID, foldersToDelete, entitiesToDestroy);

    for (EntityID entity : entitiesToDestroy)
    {
        if (IsEntityAlive(*scene, entity))
            scene->DestroyEntity(entity);
    }

    m_pendingDeleteFolder = _folderID;

    for (hierarchy::FolderID folderID : foldersToDelete)
    {
        if (folderID != folderManager.m_rootFolder)
            folderManager.DeleteFolder(folderID);
    }

    ClearSelection();
}

void WindowHierarchy::SelectFolder(hierarchy::FolderID _folderID)
{
    if (_folderID == hierarchy::INVALID_FOLDER)
        return;

    m_selectedFolder = _folderID;
    LastTypeSelectedWasFolderId = true;

    UpdateManagerSelectedItem(INVALID_ENTITY);
}