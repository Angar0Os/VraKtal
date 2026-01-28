
#include "imGuiWindows.h"
#include "contentDrawer.h"
#include "imgui/imgui.h"

#include <core/gpu/buffer.h>

#include <graphics/resources/object/camera.h>
#include <graphics/renderer.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.inl>
#include <glm/gtx/matrix_decompose.inl>

#include <imGuizmo/ImGuizmo.h>
#include <imgui/imgui.h>

#include <iostream>

ImGuiWindows::ImGuiWindows(graphics::Renderer* _renderer)
{
	m_renderer = _renderer;
}

ImGuiWindows::~ImGuiWindows()
{
}

void ImGuiWindows::PrepareImGuiWindows()
{
	//const float* viewMatrix = glm::value_ptr(m_renderer->GetViewMatrix());
 //   glm::mat4 NdcProj = m_renderer->GetProjectionMatrix();
 //   NdcProj[1][1] *= -1.0f;
	//const float* proj = glm::value_ptr(NdcProj);

	//EditTransformByIndice(viewMatrix, proj, objectIndex);

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f,0.0f));

	mainWindow();

    testWindow();
	ContentDrawerWindow();
    HierarchyWindow();
}

void ImGuiWindows::ContentDrawerWindow()
{
	m_contentDrawer.GetContentDrawerWindow();
}

void ImGuiWindows::HierarchyWindow()
{

}

void ImGuiWindows::testWindow()
{
	ImGui::Begin("Hello ImGui + Vulkan");

	ImGui::Text("If you see this, ImGui works!");
	ImGui::Separator();
	static float f = 0.0f;
	ImGui::SliderFloat("Test slider", &f, 0.0f, 1.0f);
	ImGui::Text("Value = %.3f", f);
	ImGui::End();
}

void ImGuiWindows::EditTransformByIndice(const float* cameraView, const float* cameraProjection, int objIndice)
{
    /*graphics::resources::object::Object* object = m_renderer->GetScene().get()->objects[objIndice].get();
    float* ObjectMatrix = const_cast<float*>(glm::value_ptr(object->GetTransformMatrix()));

    static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::ROTATE);
    static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);
    if (ImGui::IsKeyPressed(ImGuiKey_T))
        mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_E))
        mCurrentGizmoOperation = ImGuizmo::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R))
        mCurrentGizmoOperation = ImGuizmo::SCALE;
    if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
        mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
        mCurrentGizmoOperation = ImGuizmo::ROTATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
        mCurrentGizmoOperation = ImGuizmo::SCALE;
    float matrixTranslation[3], matrixRotation[3], matrixScale[3];
    ImGuizmo::DecomposeMatrixToComponents(ObjectMatrix, matrixTranslation, matrixRotation, matrixScale);
    ImGui::InputFloat3("Tr", matrixTranslation);
    ImGui::InputFloat3("Rt", matrixRotation);
    ImGui::InputFloat3("Sc", matrixScale);
    ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, ObjectMatrix);

    if (mCurrentGizmoOperation != ImGuizmo::SCALE)
    {
        if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
            mCurrentGizmoMode = ImGuizmo::LOCAL;
        ImGui::SameLine();
        if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
            mCurrentGizmoMode = ImGuizmo::WORLD;
    }
    static bool useSnap(false);
    if (ImGui::IsKeyPressed(ImGuiKey_S))
        useSnap = !useSnap;
    ImGui::Checkbox("##useSnap", &useSnap);
    ImGui::SameLine();

    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    ImGuizmo::Manipulate(cameraView, cameraProjection, mCurrentGizmoOperation, mCurrentGizmoMode, ObjectMatrix, NULL, NULL);

    glm::mat4 transformation = glm::make_mat4(ObjectMatrix);
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;

    glm::decompose(transformation, scale, rotation, translation, skew, perspective);

    object->transform.SetPosition(translation);
    object->transform.SetRotation(rotation);
    object->transform.SetScale(scale);*/
}


void ImGuiWindows::mainWindow()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGui::Begin("DockSpace", nullptr, window_flags);
	ImGui::PopStyleVar(3);
	
	ImGuiID dockspace_id = ImGui::GetID("DockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

	SetMenuBar();
	ImGui::End();
}

void ImGuiWindows::SetMenuBar() {
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New Project")) {}

			if (ImGui::MenuItem("Open Project")) {}

			if (ImGui::MenuItem("Save Project")) {}

			if (ImGui::MenuItem("Quit")) {}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit")) {
			if (ImGui::MenuItem("Undo (CTRL + Z)")) {}

			if (ImGui::MenuItem("Redo (CTRL + Y)")) {}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Tools")) {
			if (ImGui::MenuItem("Timeline")) {}

			if (ImGui::MenuItem("CameraSplineEditor")) {}

			if (ImGui::MenuItem("Tracy")) {}
			ImGui::EndMenu();
		}

		if (ImGui::MenuItem("Build")) {
			ImGui::OpenPopup("build_popup");
		}
		if (ImGui::BeginPopup("build_popup"))
		{
			if (ImGui::Button("Build")) {}; ImGui::SameLine();
			if (ImGui::Button("Build & Run")) {}

			ImGui::EndPopup();
		}
		ImGui::EndMenuBar();
	}
}