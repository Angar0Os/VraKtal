#include "../header/contentDrawer.h"
#include "imgui/imgui.h"

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

	for (auto& entry : std::filesystem::directory_iterator(m_currentPath))
	{
		const auto& path = entry.path();
		std::string filename = path.filename().string();

		ImGui::PushID(filename.c_str());

		// todo: refacto for avoinding continue and duplicate code

		if (entry.is_directory()) {
			ImGui::BeginGroup();

			ImGui::PushFont(largeIconFont);
			if (ImGui::Button(ICON_MDI_FOLDER, ImVec2(buttonSize, 0))) {
				m_currentPath = path;
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

			continue;
		}

		ImGui::BeginGroup();

		ImGui::PushFont(largeIconFont);
		if (ImGui::Button(ICON_MDI_ACCESS_POINT, ImVec2(buttonSize, 0))) {
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