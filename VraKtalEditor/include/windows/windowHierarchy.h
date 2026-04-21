#pragma once
#include "ImguiWindowBase.h"
#include <core/input/keys.h>

#include <vector>
#include <glm/glm.hpp>

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
	WindowHierarchy(Scene& _scene , graphics::Renderer& _renderer, ImGuiWindows& _imGuiWindows);
	~WindowHierarchy();

	void Draw() override;

	bool DrawVec3Control(const char* label, glm::vec3& value, float resetValue, float columnWidth);

	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotationDeg, glm::vec3& scale);

	bool DrawTransformEditor(const char* label, glm::mat4& transform);

	void DrawMeshInstanceProperties(timeline::MeshInstance& currentMesh);

	void DrawGuizmo(timeline::MeshInstance& object, const glm::mat4& cameraView, const glm::mat4& cameraProjection);

private:
	Scene& m_scene;
    graphics::Renderer& m_renderer;
	ImGuiWindows& m_imGuiWindows;
};

glm::mat4 ComposeTransform(const glm::vec3& translation, const glm::vec3& rotationDeg, const glm::vec3& scale);

void DrawMatrix4ReadOnly(const glm::mat4& m);
