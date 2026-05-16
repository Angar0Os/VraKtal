#pragma once

#include "ImguiWindowBase.h"
#include <core/input/keys.h>

#include <vector>
#include <glm/glm.hpp>
#include <scene/timeline/entityBase.h>
#include <unordered_map>
#include <utils/denseStorage.h>
#include <string>
#include <typeindex>
#include <utility>

#include "others/folder.h"

class ImGuiWindows;
class Scene;
class SceneManager;

namespace timeline
{
    struct MeshInstance;
}

namespace graphics
{
    class Renderer;
}

class WindowHierarchy : public ImguiWindowBase
{
public:
    WindowHierarchy(SceneManager& _sceneManager, ImGuiWindows& _imGuiWindows);
    ~WindowHierarchy();

    void Draw() override;

    void OnEntityCreatedCallBack(std::pair<EntityID, size_t> _pair);
    void OnEntityDestroyedCallBack(std::pair<EntityID, size_t> _pair);

    void CreateFolder(std::string _name);
    void RenameFolder(hierarchy::FolderID _folderID);
    void DeleteFolder(hierarchy::FolderID _folderID);
    void DeleteFolderAndContent(hierarchy::FolderID _folderID);

    hierarchy::FolderManager* GetFolderManager();

private:
    Scene* GetActiveScene();
    const Scene* GetActiveScene() const;
    EntityID GetActiveSceneID() const;

    void SyncActiveScene();
    void BindActiveSceneCallbacks();
    void ClearSelection();
    void SyncFolderManagerWithScene(Scene& _scene, hierarchy::FolderManager& _folderManager);

    hierarchy::FolderManager& GetActiveFolderManager();

private:
    void DrawEntityHierarchyItem(EntityID _ID);
    void HandleRangeSelect();
    void HandleRightClick();
    void HandleInputs();

private:
    SceneManager& m_sceneManager;
    ImGuiWindows& m_imGuiWindows;

    EntityID m_currentSceneID = INVALID_ENTITY;

    std::unordered_map<EntityID, bool> m_sceneCallbacksBound;
    std::unordered_map<EntityID, hierarchy::FolderManager> m_folderManagers;

    EntityID m_renamingEntity;
    char m_entityRenameBuffer[256] = {};

    std::unordered_map<EntityID, bool> m_entitiesSelected;

    void AddSelectedEntity(EntityID _ID);
    void RemoveEntityFromSelected(EntityID _ID);
    void SetSelectedEntity(EntityID _ID);
    bool IsEntitySelected(EntityID index);

    void UpdateManagerSelectedItem(EntityID _selectedIndex);

    std::pair<EntityID, EntityID> m_rangeSelectStartEnd;
    EntityID m_selectionAnchor = INVALID_ENTITY;

    std::vector<EntityID> ConstructSelectedEntitiesVector();

    void DrawFilterBar();

    struct TypeFilter
    {
        std::type_index type;
        std::string name;
        bool enabled = false;
    };

    std::vector<TypeFilter> m_typeFilters;
    void AddTypeFilter(std::type_index type, const std::string& name);
    bool PassTypeFilters(EntityID _ID);

    bool LastTypeSelectedWasFolderId = false;

    void DrawFolders(hierarchy::FolderID _folderID);
    void SelectFolder(hierarchy::FolderID _folderID);

    hierarchy::FolderID m_selectedFolder = hierarchy::INVALID_FOLDER;
    hierarchy::FolderID m_renamingFolder = hierarchy::INVALID_FOLDER;
    hierarchy::FolderID m_pendingDeleteFolder = hierarchy::INVALID_FOLDER;

    char m_folderRenameBuffer[256] = {};
};