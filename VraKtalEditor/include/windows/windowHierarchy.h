#pragma once
#include "ImguiWindowBase.h"
#include <core/input/keys.h>

#include <vector>
#include <glm/glm.hpp>
#include <scene/timeline/entityBase.h>
#include <unordered_map>

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

	void DrawEntityHierarchyItem(size_t indexInAlive);

private:
	Scene& m_scene;
	ImGuiWindows& m_imGuiWindows;

	size_t m_renamingEntity;
	char m_entityRenameBuffer[256] = {};

	std::unordered_map<size_t, bool> m_entitiesSelected;

	void AddSelectedEntity(size_t _ID);
	void RemoveEntity(size_t _ID);
	void SetSelectedEntity(size_t _ID);
	void SetSelectedEntityInRange(size_t _IDStart, size_t _IDEnd);
	bool IsSelectedIndex(size_t index);
	
	void UpdateManagerSelectedItem(size_t _selectedIndex);

	std::pair<size_t, size_t> m_rangeSelectStartEnd;

	size_t GetSmallestSelectedEntity();
	size_t GetBiggestSelectedEntity();

	std::vector<EntityID> ConstructSelectedEntitiesVector();
};
