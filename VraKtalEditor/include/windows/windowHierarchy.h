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
#include "others/folder.h"

class ImGuiWindows;
class Scene;

namespace timeline {
	struct MeshInstance;
}

namespace graphics
{
    class Renderer;
}

class WindowHierarchy : public ImguiWindowBase
{
public:
	WindowHierarchy(Scene& _scene, ImGuiWindows& _imGuiWindows);
	~WindowHierarchy();

	void Draw() override;

	void OnEntityCreatedCallBack(std::pair<EntityID, size_t> _pair);
	void OnEntityDestroyedCallBack(std::pair<EntityID, size_t> _pair);

	void CreateFolder(std::string _name);

private:
	void DrawEntityHierarchyItem(EntityID _ID);
	void HandleRangeSelect();
	void HandleRightClick();

private:
	Scene& m_scene;
	ImGuiWindows& m_imGuiWindows;

	size_t m_renamingEntity;
	char m_entityRenameBuffer[256] = {};

	std::unordered_map<EntityID, bool> m_entitiesSelected;

	void AddSelectedEntity(EntityID _ID);
	void RemoveEntity(EntityID _ID);
	void SetSelectedEntity(EntityID _ID);
	bool IsSelectedIndex(size_t index);
	
	void UpdateManagerSelectedItem(EntityID _selectedIndex);

	std::pair<size_t, size_t> m_rangeSelectStartEnd;
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

	hierarchy::FolderManager m_folderManager;
	void DrawFolders(hierarchy::FolderID _folderID);
	hierarchy::FolderID m_renamingFolder = hierarchy::INVALID_FOLDER;
	char m_folderRenameBuffer[256] = {};
};
