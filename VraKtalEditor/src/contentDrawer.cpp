#include "../header/contentDrawer.h"
#include "imgui/imgui.h"

#include <iostream>
#include <algorithm>

#include "MDI/IconsMaterialDesignIcons.h"

constexpr const char* baseAssetPath = "assets";

ContentDrawer::ContentDrawer() : m_currentPath(baseAssetPath), m_needsRefresh(true)
{
	m_currentPath = baseAssetPath;
}

void ContentDrawer::ClearSelection()
{
	m_selectedIndices.clear();
	for (auto& file : m_cachedFiles) {
		file.isSelected = false;
	}
}

void ContentDrawer::GetContentDrawerWindow()
{
	ImGui::Begin("Content Drawer", nullptr, ImGuiWindowFlags_MenuBar);

	if (ImGui::BeginMenuBar()) {
		if (ImGui::MenuItem(ICON_MDI_PLUS " Add")) {
			m_needsRefresh = true;
		}

		if (ImGui::MenuItem("Import")) {
			m_needsRefresh = true;
		}

		if (ImGui::MenuItem(ICON_MDI_REFRESH)) {
			m_needsRefresh = true;
		}

		std::string pathStr = m_currentPath.string();
		float pathWidth = ImGui::CalcTextSize(pathStr.c_str()).x;
		float availableWidth = ImGui::GetContentRegionAvail().x;

		if (availableWidth > pathWidth) {
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + availableWidth - pathWidth);
		}

		ImGui::TextDisabled("%s", pathStr.c_str());

		ImGui::EndMenuBar();
	}

	if (m_currentPath != baseAssetPath && ImGui::Button(ICON_MDI_ARROW_LEFT)) {
		m_currentPath = m_currentPath.parent_path();
		m_needsRefresh = true;
	}

	if (m_needsRefresh) {
		RefreshFileList();
	}

	if (ImGui::IsWindowFocused()) {
		if (ImGui::IsKeyPressed(ImGuiKey_Delete) && !m_selectedIndices.empty()) {
			m_showDeleteDialog = true;
		}
	}

	const float buttonSize = 80.0f;

	// todo : refacto the way to access fonts
	ImFont* largeIconFont = ImGui::GetIO().Fonts->Fonts[1];

	HandleFileActions();

	ShowRenameDialog();
	ShowDeleteDialog();

	if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !ImGui::IsAnyItemHovered()) {
		// todo : clipboard to handle
		ClearSelection();
		ImGui::OpenPopup("EmptySpaceMenu");
	}

	if (ImGui::BeginPopup("EmptySpaceMenu")) {
		if (ImGui::MenuItem(ICON_MDI_CONTENT_PASTE " Paste", "Ctrl+V")) {}
		ImGui::EndPopup();
	}

	if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered()) {
		ClearSelection();
	}

	for (size_t i = 0; i < m_cachedFiles.size(); ++i)
	{
		auto& fileEntry = m_cachedFiles[i];

		ImGui::PushID(static_cast<int>(i));
		ImGui::BeginGroup();

		ImVec2 groupPos = ImGui::GetCursorScreenPos();
		ImVec2 groupSize = ImVec2(buttonSize, buttonSize + ImGui::GetTextLineHeight() * 2);

		if (fileEntry.isSelected) {
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			draw_list->AddRectFilled(
				groupPos,
				ImVec2(groupPos.x + groupSize.x, groupPos.y + groupSize.y),
				IM_COL32(100, 150, 255, 100),
				4.0f
			);
		}

		ImGui::PushFont(largeIconFont);
		const char* icon = fileEntry.isDirectory ? ICON_MDI_FOLDER : GetIconForFileType(fileEntry.fileType);

		bool clicked = ImGui::Button(icon, ImVec2(buttonSize, 0));
		ImGui::PopFont();

		if (clicked) {
			if (ImGui::GetIO().KeyCtrl) {
				if (fileEntry.isSelected) {
					fileEntry.isSelected = false;
					m_selectedIndices.erase(i);
				}
				else {
					fileEntry.isSelected = true;
					m_selectedIndices.insert(i);
				}
			}
			else if (ImGui::GetIO().KeyShift && !m_selectedIndices.empty()) {
				size_t lastSelected = *m_selectedIndices.rbegin();
				size_t start = std::min(lastSelected, i);
				size_t end = std::max(lastSelected, i);

				for (size_t idx = start; idx <= end; ++idx) {
					m_cachedFiles[idx].isSelected = true;
					m_selectedIndices.insert(idx);
				}
			}
			else {
				if (fileEntry.isDirectory) {
					m_currentPath = fileEntry.path;
					m_needsRefresh = true;
					ClearSelection();
				}
				else {
					ClearSelection();
					fileEntry.isSelected = true;
					m_selectedIndices.insert(i);
				}
			}
		}

		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
			if (!fileEntry.isSelected) {
				ClearSelection();
				fileEntry.isSelected = true;
				m_selectedIndices.insert(i);
			}
			ImGui::OpenPopup("FileActionsPopup");
		}

		ImVec2 textPos = ImGui::GetCursorPos();
		ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + buttonSize);
		ImVec2 textSize = ImGui::CalcTextSize(fileEntry.filename.c_str(), nullptr, false, buttonSize);
		ImGui::SetCursorPosX(textPos.x + (buttonSize - textSize.x) * 0.5f);
		ImGui::Text("%s", fileEntry.filename.c_str());
		ImGui::PopTextWrapPos();

		ImGui::EndGroup();
		ImGui::SameLine();

		ImGui::PopID();
	}

	ImGui::End();
}

void ContentDrawer::HandleFileActions()
{
	if (m_selectedIndices.empty()) {
		return;
	}

	if (ImGui::BeginPopupContextWindow("FileActionsPopup")) {
		if (ImGui::MenuItem(ICON_MDI_PENCIL " Rename", "", false, m_selectedIndices.size() == 1)) {
			m_renameTargetIndex = *m_selectedIndices.begin();

			// Windows specific
			strncpy_s(m_renameBuffer, m_cachedFiles[m_renameTargetIndex].filename.c_str(), sizeof(m_renameBuffer) - 1);

			m_showRenameDialog = true;
		}

		if (ImGui::MenuItem(ICON_MDI_DELETE " Delete", "Del")) {
			m_showDeleteDialog = true;
		}

		if (ImGui::MenuItem(ICON_MDI_FOLDER_OPEN " Show in Explorer")) {
			for (size_t idx : m_selectedIndices) {
				if (idx < m_cachedFiles.size()) {
					std::string command = "explorer /select,\"" + m_cachedFiles[idx].path.string() + "\"";
					system(command.c_str());
				}
			}
		}

		ImGui::Separator();

		if (ImGui::MenuItem(ICON_MDI_CONTENT_COPY " Copy")) {}

		if (ImGui::MenuItem(ICON_MDI_CONTENT_CUT " Cut")) {}

		ImGui::EndPopup();
	}
}

void ContentDrawer::PerformDelete()
{
	for (size_t idx : m_selectedIndices) {
		if (idx >= m_cachedFiles.size()) {
			continue;
		}

		auto& file = m_cachedFiles[idx];
		try {
			if (file.isDirectory) {
				std::filesystem::remove_all(file.path);
			}
			else {
				std::filesystem::remove(file.path);
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Delete failed for " << file.filename << ": " << e.what() << std::endl;
		}
	}

	ClearSelection();
	m_needsRefresh = true;
}

void ContentDrawer::PerformRename()
{
	if (m_renameTargetIndex >= m_cachedFiles.size()) {
		return;
	}

	std::string newName = m_renameBuffer;
	if (newName.empty()) {
		std::cerr << "Rename failed: new name is empty" << std::endl;
		return;
	}

	auto& file = m_cachedFiles[m_renameTargetIndex];
	std::filesystem::path newPath = file.path.parent_path() / newName;

	try {
		std::filesystem::rename(file.path, newPath);
		m_needsRefresh = true;
	}
	catch (const std::exception& e) {
		std::cerr << "Rename failed: " << e.what() << std::endl;
	}
}

void ContentDrawer::RefreshFileList() {
	m_cachedFiles.clear();

	for (auto& entry : std::filesystem::directory_iterator(m_currentPath)) {
		FileEntry fileEntry;
		fileEntry.path = entry.path();
		fileEntry.filename = entry.path().filename().string();
		fileEntry.isDirectory = entry.is_directory();
		fileEntry.fileType = FileTypeDetector::DetectFileType(fileEntry.path);
		fileEntry.isSelected = false;

		std::cout << "fileType " << fileEntry.fileType << std::endl;

		m_cachedFiles.push_back(fileEntry);
	}

	m_needsRefresh = false;
}

void ContentDrawer::ShowRenameDialog()
{
	if (m_showRenameDialog) {
		ImGui::OpenPopup("Rename");
		m_showRenameDialog = false;
	}

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("Rename", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Rename: %s", m_cachedFiles[m_renameTargetIndex].filename.c_str());
		ImGui::Separator();

		ImGui::InputText("New Name", m_renameBuffer, sizeof(m_renameBuffer));

		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120, 0))) {
			PerformRename();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void ContentDrawer::ShowDeleteDialog()
{
	if (m_showDeleteDialog) {
		ImGui::OpenPopup("Delete Confirmation");
		m_showDeleteDialog = false;
	}

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("Delete Confirmation", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Are you sure you want to delete %zu item(s)?", m_selectedIndices.size());
		ImGui::Separator();

		ImGui::Text("Items to delete:");
		for (size_t idx : m_selectedIndices) {
			if (idx < m_cachedFiles.size()) {
				ImGui::BulletText("%s", m_cachedFiles[idx].filename.c_str());
			}
		}

		ImGui::Separator();

		if (ImGui::Button("Delete", ImVec2(120, 0))) {
			PerformDelete();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

const char* ContentDrawer::GetIconForFileType(FileType type)
{
	switch (type) {
	case FileType::ImagePNG:
	case FileType::ImageJPEG:
		return ICON_MDI_FILE_IMAGE;

	case FileType::AudioMP3:
	case FileType::AudioWAV:
		return ICON_MDI_FILE_MUSIC;

	case FileType::MeshOBJ:
	case FileType::MeshGLTF:
	case FileType::MeshGLB:
		return ICON_MDI_CUBE_OUTLINE;

	case FileType::Unknown:
	default:
		return ICON_MDI_FILE;
	}
}