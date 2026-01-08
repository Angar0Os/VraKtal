#include "../header/contentDrawer.h"
#include "imgui/imgui.h"

#include <iostream>

#include "MDI/IconsMaterialDesignIcons.h"

constexpr const char* baseAssetPath = "assets";

ContentDrawer::ContentDrawer() : m_currentPath(baseAssetPath), m_needsRefresh(true)
{
	m_currentPath = baseAssetPath;
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

	const float buttonSize = 80.0f;

	// todo : refacto the way to access fonts
	ImFont* largeIconFont = ImGui::GetIO().Fonts->Fonts[1];

	for (auto& fileEntry : m_cachedFiles)
	{
		ImGui::PushID(fileEntry.filename.c_str());
		ImGui::BeginGroup();

		ImGui::PushFont(largeIconFont);
		const char* icon = fileEntry.isDirectory ? ICON_MDI_FOLDER : GetIconForFileType(fileEntry.fileType);

		if (ImGui::Button(icon, ImVec2(buttonSize, 0))) {
			if (fileEntry.isDirectory) {
				m_currentPath = fileEntry.path;
				m_needsRefresh = true;
			}
		}
		ImGui::PopFont();

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

void ContentDrawer::RefreshFileList() {
	m_cachedFiles.clear();

	for (auto& entry : std::filesystem::directory_iterator(m_currentPath)) {
		FileEntry fileEntry;
		fileEntry.path = entry.path();
		fileEntry.filename = entry.path().filename().string();
		fileEntry.isDirectory = entry.is_directory();
		fileEntry.fileType = FileTypeDetector::DetectFileType(fileEntry.path);

		std::cout << "fileType " << fileEntry.fileType << std::endl;

		m_cachedFiles.push_back(fileEntry);
	}

	m_needsRefresh = false;
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