#define NOMINMAX
#include <imGuiWindows.h>
#include "command/fileCommands.h"

#include <core/gpu/imguiContext.h>
#include <core/input/input.h>
#include <graphics/renderer.h>
#include <scene/scene.h>


#include <core/gpu/buffer.h>
#include <core/manager/ressourceManager.h>

#include <imgui/imgui.h>

#include "portable-file-dialogs/portable-file-dialogs.h"

#include <MDI/IconsMaterialDesignIcons.h>

#include "contentDrawer.h"
#include "../include/windows/WindowInput.h"
#include "../include/windows/WindowViewport.h"
#include "../include/windows/windowHierarchy.h"
#include "../include/windows/windowInspector.h"

#include "../include/windows/others/imGuizmoHelper.h"
#include "../include/windows/others/inspector.h"
#include "../include/windows/others/rightClick.h"
#include "../include/windows/others/meshPlot.h"
#include "../include/windows/others/dragNdrop.h"

struct ImguiOthers {
	ImguiOthers(ImGuiWindows* _windows, core::Input* _input , RessourceManager* _reManager) 
		: imGuizmoHelper(new ImGuizmoHelper(_windows, _input)), rightClick(new RightClick(_windows)), meshPlot(new MeshPlot(_windows)) , dragNdrop(new DragNDrop(*_windows , *_reManager)) {};
	~ImguiOthers() {
		delete imGuizmoHelper;
		delete rightClick;
		delete meshPlot;
		delete dragNdrop;
	}

	ImGuizmoHelper* imGuizmoHelper;
	RightClick* rightClick;
	MeshPlot* meshPlot;
	DragNDrop* dragNdrop;
};


ImGuiWindows::ImGuiWindows(core::gpu::ImguiContext* _imGuiContext, graphics::Renderer* _renderer, core::Window* window, core::Input& _input, Scene* _scene, RessourceManager& _manager )
    : m_imGuiContext(_imGuiContext), m_scene(_scene), m_others(new ImguiOthers(this, &_input , &_manager))
{
	m_renderer = _renderer;
	m_window = window;
	m_inspect = new Inspect(&_manager);
	m_input = &_input;

	command::ClearBackupDirectory();
	m_commandHistory = std::make_unique<command::CommandHistory>(100);

	m_contentDrawer = new ContentDrawer(this);
	m_contentDrawer->SetCommandHistory(m_commandHistory.get());

	m_windows.push_back(new WindowInput(_input , this));
	m_windows.push_back(new WindowViewport(*this));
	m_windows.push_back(new WindowHierarchy(*_scene , *this));
    m_windows.push_back(new WindowInspector(*this , _manager));
}

ImGuiWindows::~ImGuiWindows()
{
	command::ClearBackupDirectory();
	delete m_contentDrawer;
	for (auto* window : m_windows) {
		delete window;
    }
    delete m_others;
}

ImGuizmoHelper* ImGuiWindows::GetImGuizmoHelper()	{ return m_others->imGuizmoHelper; };
RightClick* ImGuiWindows::GetRightClick()			{ return m_others->rightClick; };
MeshPlot* ImGuiWindows::GetMeshPlot()				{ return m_others->meshPlot; };
DragNDrop* ImGuiWindows::GetDragNDrop()				{ return m_others->dragNdrop; };


void ImGuiWindows::DrawImGui()
{
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	MainWindow();
	ContentDrawerWindow();
	for (auto& var : m_windows)
	{
        var->Draw();
	}


	//TODO : Find a way to reset selected item
	//if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered())
	//{
	//	m_selectedItem = std::monostate{};
	//}

}

core::gpu::ImguiContext* ImGuiWindows::GetContext()
{
	return m_imGuiContext;
}

void ImGuiWindows::ResetSelectedItem()
{
    m_selectedItem = std::monostate{};
}

void ImGuiWindows::ContentDrawerWindow()
{
	if (BeginWindow("Content Drawer", true, ImGuiWindowFlags_MenuBar))
	{
		m_contentDrawer->GetContentDrawerWindow();
	}
	EndWindow("Content Drawer");
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

void ImGuiWindows::MainWindow()
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

//help
glm::mat4 ImGuiWindows::GetView()
{
	return m_renderer->GetViewMatrix();
}