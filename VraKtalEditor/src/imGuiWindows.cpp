#define NOMINMAX
#include "imGuiWindows.h"
#include <core/gpu/imguiContext.h>
#include "command/fileCommands.h"
#include <graphics/resources/object/camera.h>
#include <algorithm>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.inl>
#include <glm/gtx/matrix_decompose.inl>

#include <imGuizmo/ImGuizmo.h>
#include <imgui/imgui.h>

#include "portable-file-dialogs/portable-file-dialogs.h"

#include <iostream>
#include <MDI/IconsMaterialDesignIcons.h>

#include <core/input/input.h>

#include "contentDrawer.h"
#include "../include/windows/WindowInput.h"
#include "../include/windows/WindowViewport.h"

ImGuiWindows::ImGuiWindows(core::gpu::ImguiContext* _imGuiContext, graphics::Renderer* _renderer, core::Window* window , core::Input& _input)
    : m_imGuiContext(_imGuiContext)
{
    m_renderer = _renderer;
    m_window = window;

    command::ClearBackupDirectory();

    m_commandHistory = std::make_unique<command::CommandHistory>(100);
    
	m_contentDrawer = new ContentDrawer();
	m_contentDrawer->SetCommandHistory(m_commandHistory.get());

	m_windowInput = new WindowInput(_input);
	m_windowViewport = new WindowViewport(*this);

}

ImGuiWindows::~ImGuiWindows()
{
	command::ClearBackupDirectory();
	
	delete m_contentDrawer;
	delete m_windowInput;
}

void ImGuiWindows::PrepareImGuiWindows()
{
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

	m_windowViewport->Draw();
	m_windowInput->Draw();

	m_newProjectModal.GetNewProjectModalWindow();

	if (m_newProjectModal.HasNewProjectCreated()) {
		std::filesystem::path lastProjectPath = m_newProjectModal.GetLastCreatedProjectPath();

		if (!lastProjectPath.empty()) {
			m_contentDrawer->SetCurrentPath(lastProjectPath);
		}

		m_newProjectModal.ResetProjectCreatedFlag();
	}
}

core::gpu::ImguiContext* ImGuiWindows::GetContext()
{
	return m_imGuiContext;
}

void ImGuiWindows::ContentDrawerWindow()
{
	m_contentDrawer->GetContentDrawerWindow();
	if (BeginWindow("Content Drawer", true, ImGuiWindowFlags_MenuBar))
	{
		m_contentDrawer.GetContentDrawerWindow();
	}
	EndWindow("Content Drawer");
}

void ImGuiWindows::HierarchyWindow()
{

}

void ImGuiWindows::testWindow()
{
	

}

void ImGuiWindows::AddWindowToManager(const std::string& name, bool windowState)
{
	m_windowStatesList[name].isOpen = windowState;
	m_windowStatesList[name].keepOpen = true;
}

bool ImGuiWindows::BeginWindow(const std::string& name, bool defaultStateIfNotExists, ImGuiWindowFlags flags)
{
	if (!m_windowStatesList.contains(name))
	{
		AddWindowToManager(name, defaultStateIfNotExists);
	}

	if (m_windowStatesList[name].isOpen)
	{
		return ImGui::Begin(name.c_str(), &m_windowStatesList[name].keepOpen, flags);
	}

	return false;
}

void ImGuiWindows::EndWindow(const std::string& name)
{
	if (!m_windowStatesList.contains(name))
	{
		return;
	}

	if (m_windowStatesList[name].isOpen)
	{
		ImGui::End();
	}

	if (!m_windowStatesList[name].keepOpen)
	{
		m_windowStatesList[name].isOpen = false;
	}

	m_windowStatesList[name].keepOpen = true;
}

void ImGuiWindows::DisplayWindowStateManagerMenu()
{
	if (ImGui::BeginMenu("Windows"))
	{
		for (auto& [name, status] : m_windowStatesList)
		{
			ImGui::MenuItem(name.c_str(), nullptr, &status.isOpen);
		}
		ImGui::EndMenu();
	}
}

void ImGuiWindows::Viewport()
{
	if (BeginWindow("Viewport"))
	{
		ImVec2 avail = ImGui::GetContentRegionAvail();

		uint32_t width = std::max(1u, static_cast<uint32_t>(avail.x));
		uint32_t height = std::max(1u, static_cast<uint32_t>(avail.y));

		m_imGuiContext->DrawViewportComponent(width, height);
	}
	EndWindow("Viewport");
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
			if (ImGui::MenuItem("New Project")) {
				m_newProjectModal.ToggleNewProjectModal();
			}

			if (ImGui::MenuItem("Open Project")) {
				ImGuiWindows::LoadProject();
			}

			if (ImGui::MenuItem("Save Project")) {}

			if (ImGui::MenuItem("Quit")) {
				if (m_window) {
					m_window->Close();
				}
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit")) {
			bool canUndo = m_commandHistory->CanUndo();
			std::string undoText = "Undo";

			if (canUndo) {
				undoText += " : " + m_commandHistory->GetCommandDescription(m_commandHistory->GetCurrentIndex());
			}

			if (ImGui::MenuItem((ICON_MDI_UNDO " " + undoText).c_str(), "Ctrl + Z", false, canUndo)) {
				m_commandHistory->Undo();
			}

			bool canRedo = m_commandHistory->CanRedo();
			std::string redoText = "Redo";

			if (canRedo) {
				redoText += " : " + m_commandHistory->GetCommandDescription(m_commandHistory->GetCurrentIndex() + 1);
			}

			if (ImGui::MenuItem((ICON_MDI_REDO " " + redoText).c_str(), "Ctrl + Y", false, canRedo)) {
				m_commandHistory->Redo();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Tools")) {
			if (ImGui::MenuItem("Timeline")) {}

			if (ImGui::MenuItem("CameraSplineEditor")) {}

			if (ImGui::MenuItem("Tracy")) {}
			ImGui::EndMenu();
		}

		DisplayWindowStateManagerMenu();

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

void ImGuiWindows::LoadProject()
{
	auto selection = pfd::open_file(
		"Choose a project",
		"",
		{
			"YAML", "*.yaml",
		}
		);

	auto files = selection.result();

	if (files.empty()) {
		return;
	}

	std::filesystem::path projectPath(files[0]);

	if (!projectPath.parent_path().empty()) {
		m_contentDrawer->SetCurrentPath(projectPath.parent_path());
	}

	// Global shortcuts
	if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z)) {
		if (m_commandHistory->CanUndo()) {
			m_commandHistory->Undo();
		}
	}
	if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y)) {
		if (m_commandHistory->CanRedo()) {
			m_commandHistory->Redo();
		}
	}
}