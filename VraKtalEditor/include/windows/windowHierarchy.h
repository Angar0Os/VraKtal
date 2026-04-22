#pragma once
#include "ImguiWindowBase.h"
#include <core/input/keys.h>

#include <vector>
#include <glm/glm.hpp>
#include <scene/timeline/entityBase.h>

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
	WindowHierarchy(Scene& _scene , ImGuiWindows& _imGuiWindows);
	~WindowHierarchy();

	void Draw() override;

	void DrawEntityHierarchyItem(EntityID ID);

private:
	Scene& m_scene;
	ImGuiWindows& m_imGuiWindows;

	EntityID m_editingEntity;
	char m_entityRenameBuffer[256] = {};
};
