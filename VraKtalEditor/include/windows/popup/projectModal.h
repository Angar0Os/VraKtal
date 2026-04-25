#ifndef EDITOR_WINDOWS_NEWPROJECTMODAL_H
#define EDITOR_WINDOWS_NEWPROJECTMODAL_H
#pragma once

#include <filesystem>
#include <string>

class ProjectModal
{
private:
	char m_nameBuffer[256] = "";
	bool m_showDialog = false;
	std::string m_selectionPath;
	std::filesystem::path m_lastCreateProjectPath;
	bool m_projectCreated = false;

	void CreateProject();

public:
	ProjectModal();

	std::filesystem::path GetLastCreatedProjectPath() const { return m_lastCreateProjectPath; }
	bool HasNewProjectCreated() const { return m_projectCreated; }
	void ResetProjectCreatedFlag() { m_projectCreated = false; }

	void ToggleNewProjectModal();
	void Draw();
};

#endif //EDITOR_WINDOWS_NEWPROJECTMODAL_H