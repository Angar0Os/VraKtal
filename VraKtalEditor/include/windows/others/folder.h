#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <scene/timeline/entityBase.h>
#include <unordered_map>

namespace hierarchy {
	using FolderID = uint32_t;
	constexpr FolderID INVALID_FOLDER = UINT32_MAX;

	struct Folder
	{
		FolderID id = INVALID_FOLDER;
		std::string name;

		FolderID parent = INVALID_FOLDER;

		std::vector<FolderID> children;
		std::vector<EntityID> entities;

		bool open = true;
	};

	struct FolderManager {
		FolderManager(){InitFolders();};

		FolderID CreateFolder(const std::string& _name, FolderID _parent);
		Folder& GetFolder(FolderID _id);

		void MoveEntityToFolder(EntityID entity, FolderID targetFolder);
		void MoveFolderToFolder(FolderID _idMoving, FolderID _idTarget);
		void DeleteFolder(FolderID _folderID);

		void RemoveEntityFromFolder(EntityID entity);
		void RemoveEntityFromAllFolders(EntityID entity);

		void CollectVisibleEntities(FolderID folderID, std::vector<EntityID>& out);
		void CollectFolderAsOneBlock(FolderID folderID, std::vector<EntityID>& out);

		std::vector<EntityID> GetEntitiesInRange(EntityID _start, EntityID _end);

		std::vector<Folder> m_folders; //tout les folders 
		std::unordered_map<EntityID, FolderID> m_entityFolder;

		FolderID m_rootFolder = 0;
		FolderID m_nextFolderID = 1;
		
		//c'est frustrant , c'est sur qu'on peut faire mieux

	private:
		void InitFolders();
	};
}