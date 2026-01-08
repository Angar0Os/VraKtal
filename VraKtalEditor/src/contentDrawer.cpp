#include "../header/contentDrawer.h"
#include "imgui/imgui.h"

#include <iostream>

#include "MDI/IconsMaterialDesignIcons.h"

constexpr const char* baseAssetPath = "assets";

ContentDrawer::ContentDrawer()
{
	m_currentPath = baseAssetPath;
}

void ContentDrawer::GetContentDrawerWindow()
{
	ImGui::Begin("Content Drawer", nullptr, ImGuiWindowFlags_MenuBar);

	if (ImGui::BeginMenuBar()) {
		if (ImGui::MenuItem(ICON_MDI_PLUS " Add")) {}

		if (ImGui::MenuItem("Import")) {}

		ImGui::EndMenuBar();
	}

	if (m_currentPath != baseAssetPath && ImGui::Button(ICON_MDI_ARROW_LEFT)) {
		m_currentPath = m_currentPath.parent_path();
	}

	const float buttonSize = 80.0f;

	// todo : refacto the way to access fonts
	ImFont* largeIconFont = ImGui::GetIO().Fonts->Fonts[1];

	// todo: check because is inf. loop here !
	for (auto& entry : std::filesystem::directory_iterator(m_currentPath))
	{
		const auto& path = entry.path();
		std::string filename = path.filename().string();
		bool isDirectory = entry.is_directory();

		FileType fileType = FileTypeDetector::DetectFileType(path);

		std::cout << "fileType " << fileType << std::endl;

		ImGui::PushID(filename.c_str());
		ImGui::BeginGroup();

		ImGui::PushFont(largeIconFont);
		const char* icon = isDirectory ? ICON_MDI_FOLDER : GetIconForFileType(fileType);

		if (ImGui::Button(icon, ImVec2(buttonSize, 0))) {
			if (isDirectory) {
				m_currentPath = path;
			}
		}
		ImGui::PopFont();

		ImVec2 textPos = ImGui::GetCursorPos();
		ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + buttonSize);
		ImVec2 textSize = ImGui::CalcTextSize(filename.c_str(), nullptr, false, buttonSize);
		ImGui::SetCursorPosX(textPos.x + (buttonSize - textSize.x) * 0.5f);
		ImGui::Text("%s", filename.c_str());
		ImGui::PopTextWrapPos();

		ImGui::EndGroup();
		ImGui::SameLine();

		ImGui::PopID();
	}

	ImGui::End();
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