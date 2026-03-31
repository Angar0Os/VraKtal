#include "contentDrawer.h"
#include "imgui/imgui.h"

// Define NOMINMAX before including portable-file-dialogs to prevent Windows min/max macros
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "portable-file-dialogs/portable-file-dialogs.h"

#include <iostream>
#include <algorithm>

#include "MDI/IconsMaterialDesignIcons.h"
#include <command/fileCommands.h>

constexpr const char* baseAssetPath = "assets";

ContentDrawer::ContentDrawer() : m_currentPath(baseAssetPath), m_needsRefresh(true)
{
	try {
		m_currentPath = std::filesystem::absolute(baseAssetPath);
		m_baseAssetPath = m_currentPath;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to initialize ContentDrawer: " << e.what() << std::endl;

		m_currentPath = std::filesystem::current_path();
		m_baseAssetPath = m_currentPath;
	}
}

ContentDrawer::~ContentDrawer()
{

}

void ContentDrawer::SetCommandHistory(command::CommandHistory* history)
{
	m_commandHistory = history;

	if (m_commandHistory) {
		m_commandHistory->SetOnHistoryChangedCallback([this]() {
			m_needsRefresh = true;
		});
	}
}

void ContentDrawer::ClearSelection()
{
	m_selectedIndices.clear();
	for (auto& file : m_cachedFiles) {
		file.isSelected = false;
	}

	m_renameTargetIndex = 0;
}

void ContentDrawer::GetContentDrawerWindow()
{
	ImGui::Begin("Content Drawer", nullptr, ImGuiWindowFlags_MenuBar);

	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu(ICON_MDI_PLUS " Add")) {
			if (ImGui::MenuItem(ICON_MDI_FOLDER " Folder")) {
				PerformCreateFolder();
			}

			ImGui::EndMenu();

			m_needsRefresh = true;
		}

		if (ImGui::MenuItem("Import")) {
			PerformImport();
		}

		if (ImGui::MenuItem(ICON_MDI_REFRESH)) {
			m_needsRefresh = true;
		}

		std::filesystem::path tempPath = m_currentPath;
		std::vector<std::pair<std::string, std::filesystem::path>> breadcrumbs;

		while (!tempPath.empty()) {
			breadcrumbs.insert(breadcrumbs.begin(), { tempPath.filename().string(), tempPath });

			if (tempPath == m_baseAssetPath || tempPath == tempPath.root_path()) {
				break;
			}

			std::filesystem::path parentPath = tempPath.parent_path();
			if (parentPath == tempPath) {
				break;
			}

			tempPath = parentPath;
		}

		float breadcrumbWidth = 0.0f;
		for (size_t i = 0; i < breadcrumbs.size(); ++i)
		{
			breadcrumbWidth += ImGui::CalcTextSize(breadcrumbs[i].first.c_str()).x;
			if (i > 0) {
				breadcrumbWidth += ImGui::CalcTextSize(" / ").x;
			}
		}

		breadcrumbWidth += ImGui::GetStyle().ItemSpacing.x * (breadcrumbs.size() - 1);

		float availableWidth = ImGui::GetContentRegionAvail().x;
		if (availableWidth > breadcrumbWidth) {
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + availableWidth - breadcrumbWidth);
		}

		for (size_t i = 0; i < breadcrumbs.size(); ++i) {
			if (i > 0) {
				ImGui::SameLine();
				ImGui::TextDisabled("/");
				ImGui::SameLine();
			}

			if (i == breadcrumbs.size() - 1) {
				ImGui::TextDisabled("%s", breadcrumbs[i].first.c_str());
			}
			else {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.7f, 1.0f, 1.0f));
				ImGui::Text("%s", breadcrumbs[i].first.c_str());
				ImGui::PopStyleColor();

				if (ImGui::IsItemHovered()) {
					ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.8f, 1.0f, 1.0f));
					ImGui::PopStyleColor();
				}

				if (ImGui::IsItemClicked()) {
					m_currentPath = breadcrumbs[i].second;
					m_needsRefresh = true;
					ClearSelection();
				}
			}
		}

		ImGui::EndMenuBar();
	}

	if (m_currentPath != m_baseAssetPath && ImGui::Button(ICON_MDI_ARROW_LEFT)) {
		std::filesystem::path parentPath = m_currentPath.parent_path();
		if (!parentPath.empty() && parentPath != m_currentPath) {
			m_currentPath = parentPath;
			m_needsRefresh = true;
		}
	}

	if (m_needsRefresh) {
		RefreshFileList();
	}

	if (ImGui::IsWindowFocused()) {
		if (ImGui::IsKeyPressed(ImGuiKey_Delete) && !m_selectedIndices.empty()) {
			m_showDeleteDialog = true;
		}
		if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_C) && !m_selectedIndices.empty()) {
			PerformCopy();
		}
		if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_X) && !m_selectedIndices.empty()) {
			PerformCut();
		}
		if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_V) && !m_clipboardPaths.empty()) {
			PerformPaste();
		}
	}

	const float buttonSize = 80.0f;

	// todo : refacto the way to access fonts
	ImFont* largeIconFont = ImGui::GetIO().Fonts->Fonts[1];

	HandleFileActions();

	ShowRenameDialog();
	ShowDeleteDialog();

	if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !ImGui::IsAnyItemHovered()) {
		if (!m_clipboardPaths.empty()) {
			ImGui::OpenPopup("EmptySpaceMenu");
		}
	}

	if (ImGui::BeginPopup("EmptySpaceMenu")) {
		if (ImGui::MenuItem(ICON_MDI_CONTENT_PASTE " Paste", "Ctrl+V")) {
			PerformPaste();
		}
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

void ContentDrawer::SetCurrentPath(std::filesystem::path newPath)
{
	try {
		std::filesystem::path absolutePath = std::filesystem::absolute(newPath / baseAssetPath);

		if (!std::filesystem::exists(absolutePath)) {
			std::cerr << "Path does not exist: " << absolutePath << std::endl;
			return;
		}

		if (!std::filesystem::is_directory(absolutePath)) {
			std::cerr << "Path is not a directory: " << absolutePath << std::endl;
			return;
		}

		m_currentPath = absolutePath;
		m_baseAssetPath = absolutePath;
		m_needsRefresh = true;
		ClearSelection();
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to set current path: " << e.what() << std::endl;
	}
}

void ContentDrawer::HandleFileActions()
{
	if (m_selectedIndices.empty()) {
		return;
	}

	if (ImGui::BeginPopupContextWindow("FileActionsPopup")) {
		if (ImGui::MenuItem(ICON_MDI_PENCIL " Rename", "", false, m_selectedIndices.size() == 1)) {
			m_renameTargetIndex = *m_selectedIndices.begin();

			if (m_renameTargetIndex < m_cachedFiles.size()) {
				// Windows specific
				strncpy_s(m_renameBuffer, m_cachedFiles[m_renameTargetIndex].filename.c_str(), sizeof(m_renameBuffer) - 1);
				
				m_showRenameDialog = true;
			}

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

		if (ImGui::MenuItem(ICON_MDI_CONTENT_COPY " Copy", "Ctrl+C")) {
			PerformCopy();
		}

		if (ImGui::MenuItem(ICON_MDI_CONTENT_CUT " Cut", "Ctrl+X")) {
			PerformCut();
		}

		if (ImGui::MenuItem(ICON_MDI_CONTENT_PASTE " Paste", "Ctrl+V", false, !m_clipboardPaths.empty())) {
			PerformPaste();
		}

		ImGui::EndPopup();
	}
}

void ContentDrawer::PerformCopy()
{
	m_clipboardPaths.clear();
	m_clipboardAction = ClipboardAction::Copy;

	for (size_t idx : m_selectedIndices) {
		if (idx < m_cachedFiles.size()) {
			m_clipboardPaths.push_back(m_cachedFiles[idx].path);
		}
	}
}

void ContentDrawer::PerformCreateFolder()
{
	if (!m_commandHistory) {
		std::cerr << "Command history not set!" << std::endl;
		return;
	}

	std::filesystem::path newFolderPath = m_currentPath / "New folder";

	auto cmd = std::make_unique<command::CreateFolderCommand>(newFolderPath);
	m_commandHistory->ExecuteCommand(std::move(cmd));

	m_needsRefresh = true;
}

void ContentDrawer::PerformCut()
{
	m_clipboardPaths.clear();
	m_clipboardAction = ClipboardAction::Cut;

	for (size_t idx : m_selectedIndices) {
		if (idx < m_cachedFiles.size()) {
			m_clipboardPaths.push_back(m_cachedFiles[idx].path);
		}
	}
}

void ContentDrawer::PerformPaste()
{
	if (!m_commandHistory) {
		std::cerr << "Command history not set!" << std::endl;
		return;
	}

	if (m_clipboardPaths.empty() || m_clipboardAction == ClipboardAction::None) {
		return;
	}

	if (m_clipboardAction == ClipboardAction::Copy) {
		auto cmd = std::make_unique<command::CopyFileCommand>(m_clipboardPaths, m_currentPath);
		m_commandHistory->ExecuteCommand(std::move(cmd));
	}
	else if (m_clipboardAction == ClipboardAction::Cut) {
		auto cmd = std::make_unique<command::MoveFileCommand>(m_clipboardPaths, m_currentPath);
		m_commandHistory->ExecuteCommand(std::move(cmd));

		m_clipboardPaths.clear();
		m_clipboardAction = ClipboardAction::None;
	}

	m_needsRefresh = true;
}

void ContentDrawer::PerformDelete()
{
	if (!m_commandHistory) {
		std::cerr << "Command history not set!" << std::endl;
		return;
	}

	std::vector<std::filesystem::path> pathsToDelete;

	for (size_t idx : m_selectedIndices) {
		if (idx < m_cachedFiles.size()) {
			pathsToDelete.push_back(m_cachedFiles[idx].path);
		}
	}

	auto cmd = std::make_unique<command::DeleteFileCommand>(pathsToDelete);
	m_commandHistory->ExecuteCommand(std::move(cmd));

	ClearSelection();
	m_needsRefresh = true;
}

void ContentDrawer::PerformImport()
{
	if (!m_commandHistory) {
		std::cerr << "Command history not set!" << std::endl;
		return;
	}


	auto selection = pfd::open_file(
		"Import Files",
		"",
		{
			"All Files", "*",
			"Images", "*.png *.jpg *.jpeg",
			"Audio", "*.mp3 *.wav",
			"3D Models", "*.obj *.gltf *.glb",
		},
		pfd::opt::multiselect
		);

	auto files = selection.result();

	if (files.empty()) {
		return;
	}

	std::vector<std::filesystem::path> sourcePaths;
	for (const auto& file : files) {
		sourcePaths.push_back(std::filesystem::path(file));
	}

	auto cmd = std::make_unique<command::ImportFileCommand>(sourcePaths, m_currentPath);
	m_commandHistory->ExecuteCommand(std::move(cmd));

	m_needsRefresh = true;
}

void ContentDrawer::PerformRename()
{
	if (!m_commandHistory) {
		std::cerr << "Command history not set!" << std::endl;
		return;
	}

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

	auto cmd = std::make_unique<command::RenameFileCommand>(file.path, newPath);
	m_commandHistory->ExecuteCommand(std::move(cmd));

	m_needsRefresh = true;
}

void ContentDrawer::RefreshFileList() {
	m_cachedFiles.clear();

	try {
		if (!std::filesystem::exists(m_currentPath)) {
			std::cerr << "Directory does not exist: " << m_currentPath << std::endl;
			m_needsRefresh = false;
			return;
		}

		if (!std::filesystem::is_directory(m_currentPath)) {
			std::cerr << "Path is not a directory: " << m_currentPath << std::endl;
			m_needsRefresh = false;
			return;
		}

		for (auto& entry : std::filesystem::directory_iterator(m_currentPath)) {
			FileEntry fileEntry;
			fileEntry.path = entry.path();
			fileEntry.filename = entry.path().filename().string();
			fileEntry.isDirectory = entry.is_directory();
			fileEntry.fileType = FileTypeDetector::DetectFileType(fileEntry.path);
			fileEntry.isSelected = false;

			m_cachedFiles.push_back(fileEntry);
		}
	}
	catch (const std::filesystem::filesystem_error& e) {
		std::cerr << "Error refreshing file list: " << e.what() << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Unexpected error refreshing file list: " << e.what() << std::endl;
	}

	ClearSelection();

	m_needsRefresh = false;
}

void ContentDrawer::ShowRenameDialog()
{
	if (m_showRenameDialog) 
	{
		if (m_renameTargetIndex >= m_cachedFiles.size()) 
		{
			m_showRenameDialog = false;
			std::cerr << "Error: Cannot rename - file no longer exists" << std::endl;
			return;
		}

		ImGui::OpenPopup("Rename");
		m_showRenameDialog = false;
	}

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("Rename", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) 
	{
		if (m_renameTargetIndex >= m_cachedFiles.size()) 
		{
			ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
			return;
		}

		ImGui::Text("Rename: %s", m_cachedFiles[m_renameTargetIndex].filename.c_str());
		ImGui::Separator();

		ImGui::InputText("New Name", m_renameBuffer, sizeof(m_renameBuffer));

		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120, 0)) || ImGui::IsKeyPressed(ImGuiKey_Enter))
		{
			PerformRename();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
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

		if (ImGui::Button("Delete", ImVec2(120, 0)) || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
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