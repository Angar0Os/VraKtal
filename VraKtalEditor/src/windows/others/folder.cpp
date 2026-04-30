#include "../../../include/windows/others/folder.h"
#include <assert.h>
#include <imgui/imgui.h>
#include <iostream>

namespace hierarchy {

    void FolderManager::InitFolders()
    {
        m_folders.push_back({
        .id = m_rootFolder,
        .name = "Root",
        .parent = INVALID_FOLDER
            });
    }

    FolderID FolderManager::CreateFolder(const std::string& _name, FolderID _parent)
    {
        FolderID id = m_nextFolderID++;

        m_folders.push_back({
            .id = id,
            .name = _name,
            .parent = _parent
            });

        GetFolder(_parent).children.push_back(id);
        m_folderParent[id] = _parent;
        return id;
    }

    Folder& FolderManager::GetFolder(FolderID _id)
    {
        for (auto& folder : m_folders)
        {
            if (folder.id == _id)
                return folder;
        }

        assert(false && "Invalid folder ID");
        return m_folders[0];
    }

    void FolderManager::MoveEntityToFolder(EntityID entity, FolderID targetFolder)
    {
        std::cout << "Moving Entity: " << entity << " to folder: " << targetFolder << std::endl;
        if (m_entityFolder.contains(entity))
        {
            FolderID oldFolderID = m_entityFolder[entity];
            auto& oldEntities = GetFolder(oldFolderID).entities;

            oldEntities.erase(
                std::remove(oldEntities.begin(), oldEntities.end(), entity),
                oldEntities.end()
            );
        }
        GetFolder(targetFolder).entities.push_back(entity);
        m_entityFolder[entity] = targetFolder;
    }

    void FolderManager::MoveFolderToFolder(FolderID _idMoving, FolderID _idTarget)
    {
        if (_idMoving == INVALID_FOLDER || _idTarget == INVALID_FOLDER)
            return;
        if (_idMoving == m_rootFolder)
            return; // Root 
        if (_idMoving == _idTarget)
            return;

        Folder& movingFolder = GetFolder(_idMoving);
        Folder& targetFolder = GetFolder(_idTarget);

        FolderID parentCheck = _idTarget;
        while (parentCheck != INVALID_FOLDER)
        {
            if (parentCheck == _idMoving)
                return;
            parentCheck = GetFolder(parentCheck).parent;
        }

        if (movingFolder.parent != INVALID_FOLDER)
        {
            Folder& oldParent = GetFolder(movingFolder.parent);

            oldParent.children.erase(
                std::remove(oldParent.children.begin(), oldParent.children.end(), _idMoving),
                oldParent.children.end()
            );
        }

        if (std::find(targetFolder.children.begin(), targetFolder.children.end(), _idMoving) == targetFolder.children.end())
        {
            targetFolder.children.push_back(_idMoving);
        }
        movingFolder.parent = _idTarget;
    }

    void FolderManager::DeleteFolder(FolderID folderID)
    {
        if (folderID == INVALID_FOLDER || folderID == m_rootFolder)
            return;

        Folder& folder = GetFolder(folderID);

        // Déplacer les entités du folder vers le parent
        if (folder.parent != INVALID_FOLDER)
        {
            Folder& parent = GetFolder(folder.parent);

            for (EntityID entity : folder.entities)
            {
                parent.entities.push_back(entity);
                m_entityFolder[entity] = parent.id;
            }

            // Déplacer les enfants vers le parent
            for (FolderID childID : folder.children)
            {
                Folder& child = GetFolder(childID);
                child.parent = parent.id;
                parent.children.push_back(childID);
            }

            // Retirer le folder supprimé des children du parent
            parent.children.erase(
                std::remove(parent.children.begin(), parent.children.end(), folderID),
                parent.children.end()
            );
        }

        // Supprimer le folder du vector
        m_folders.erase(
            std::remove_if(
                m_folders.begin(),
                m_folders.end(),
                [folderID](const Folder& f)
                {
                    return f.id == folderID;
                }
            ),
            m_folders.end()
        );
    }

    void FolderManager::RemoveEntityFromFolder(EntityID entity)
    {
        auto it = m_entityFolder.find(entity);
        if (it == m_entityFolder.end())
            return;

        FolderID folderID = it->second;
        Folder& folder = GetFolder(folderID);

        std::erase(folder.entities, entity); // C++20

        m_entityFolder.erase(it);
    }

    void FolderManager::RemoveEntityFromAllFolders(EntityID entity)
    {
        for (Folder& folder : m_folders)
        {
            folder.entities.erase(
                std::remove(folder.entities.begin(), folder.entities.end(), entity),
                folder.entities.end()
            );
        }

        m_entityFolder.erase(entity);
    }

    std::vector<EntityID> FolderManager::GetEntitiesInRange(EntityID start, EntityID end)
    {
        std::vector<EntityID> visibleOrder;
        CollectVisibleEntities(m_rootFolder, visibleOrder);

        auto startIt = std::find(visibleOrder.begin(), visibleOrder.end(), start);
        auto endIt = std::find(visibleOrder.begin(), visibleOrder.end(), end);

        if (startIt == visibleOrder.end() || endIt == visibleOrder.end())
            return {};

        if (startIt > endIt)
            std::swap(startIt, endIt);

        return std::vector<EntityID>(startIt, endIt + 1);
    }

    void FolderManager::CollectVisibleEntities(FolderID folderID, std::vector<EntityID>& out)
    {
        Folder& folder = GetFolder(folderID);

        for (FolderID childID : folder.children)
        {
            Folder& child = GetFolder(childID);

            if (child.open)
                CollectVisibleEntities(childID, out);
            else
                CollectFolderAsOneBlock(childID, out);
        }

        for (EntityID entity : folder.entities)
            out.push_back(entity);
    }

    void FolderManager::CollectFolderAsOneBlock(FolderID folderID, std::vector<EntityID>& out)
    {
        Folder& folder = GetFolder(folderID);

        for (FolderID childID : folder.children)
            CollectFolderAsOneBlock(childID, out);

        for (EntityID entity : folder.entities)
            out.push_back(entity);
    }

}