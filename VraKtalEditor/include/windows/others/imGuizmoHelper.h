#pragma once
#include <vector>
#include <imGuizmo/ImGuizmo.h>
#include <glm/glm.hpp>
#include <glm/fwd.hpp>

#include "../../imGuiWindows.h"
namespace core {
	class Input;
	namespace gpu {
		class ImguiContext;
	}
}

struct GuizmoSettings
{
	ImGuizmo::OPERATION currentOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE currentMode = ImGuizmo::WORLD;
	bool bUseSnap = false;

	void SetOperationToTranslate()	{ currentOperation = ImGuizmo::TRANSLATE; };
	void SetOperationToRotate()		{ currentOperation = ImGuizmo::ROTATE; };
	void SetOperationToScale()		{ currentOperation = ImGuizmo::SCALE; };
};

class ImGuizmoHelper
{
public:

	ImGuizmoHelper(ImGuiWindows* _imGuiWindows, core::Input* _input);
	~ImGuizmoHelper();

	void DrawGuizmo();
	void AddMatriceToEdit(glm::mat4* _matrice);

private:
	ImGuiWindows& m_imGuiWindows;

	GuizmoSettings m_settings;
	std::vector<glm::mat4*> m_matrices;
};