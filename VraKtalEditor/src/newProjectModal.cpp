#include "newProjectModal.h"
#include "imgui/imgui.h"

#include "portable-file-dialogs/portable-file-dialogs.h"

#include <fstream>

NewProjectModal::NewProjectModal() {}

void NewProjectModal::GetNewProjectModalWindow()
{
	if (m_showDialog) {
		ImGui::OpenPopup("New Project");
		m_selectionPath = "";
		memset(m_nameBuffer, 0, 256);
		m_lastCreateProjectPath.clear();
		m_projectCreated = false;
		m_showDialog = false;
	}

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("New Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Separator();

		ImGui::InputText("Project Name", m_nameBuffer, sizeof(m_nameBuffer));

		if (ImGui::Button("Select Destination", ImVec2(150, 0))) {
			auto selection = pfd::select_folder("Select a folder").result();
			if (!selection.empty()) {
				std::cout << "User selected folder " << selection << "\n";
				m_selectionPath = selection;
			}
		}
		ImGui::SameLine();
		ImGui::Text(m_selectionPath.empty() ? "Choosen folder path ..." : m_selectionPath.c_str());

		ImGui::Separator();

		if ((ImGui::Button("Create", ImVec2(150, 0)) || ImGui::IsKeyPressed(ImGuiKey_Enter)) && (!m_selectionPath.empty() && m_nameBuffer[0] != '\0')) {
			NewProjectModal::CreateProject();

			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(150, 0))) {
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void NewProjectModal::ToggleNewProjectModal()
{
	m_showDialog = !m_showDialog;
}

void NewProjectModal::CreateProject()
{
	try {
		std::filesystem::path newFolderPath = std::filesystem::path(m_selectionPath) / m_nameBuffer;

		if (!std::filesystem::exists(newFolderPath)) {
			std::filesystem::create_directory(newFolderPath);
			std::filesystem::create_directory((newFolderPath / "assets"));
		}
		else {
			std::cerr << "Project creation failed: folder already exist" << std::endl;

			return;
		}

		std::filesystem::path yamlFilePath = newFolderPath / (std::format("{}.yaml", m_nameBuffer));
		std::ofstream yamlFile(yamlFilePath);

		if (!yamlFile.is_open()) {
			std::cerr << "Failed to create project at: " << yamlFilePath << std::endl;

			return;
		}

		// TODO : move this elsewhere (dedicated template yaml file?)
		yamlFile << "project:\n";
		yamlFile << "  scenes:\n";
		yamlFile << "    - name: \"Default Scene\"\n";
		yamlFile << "      objects:\n";

		yamlFile.close();

		std::cout << "Project has been created successfully at: " << yamlFilePath << std::endl;

		m_lastCreateProjectPath = newFolderPath;
		m_projectCreated = true;
	}
	catch (const std::exception& e) {
		std::cerr << "Project creation failed: " << e.what() << std::endl;
	}
}