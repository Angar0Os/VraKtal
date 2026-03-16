#ifndef EDITOR_WINDOWS_NEWPROJECTMODAL_H
#define EDITOR_WINDOWS_NEWPROJECTMODAL_H
#pragma once

#include <filesystem>
#include <string>

class NewProjectModal
{
private:
	char m_nameBuffer[256] = "";
	bool m_showDialog = false;
	std::string m_selectionPath;
	std::filesystem::path m_lastCreateProjectPath;
	bool m_projectCreated = false;

	void CreateProject();

public:
	NewProjectModal();

	std::filesystem::path GetLastCreatedProjectPath() const { return m_lastCreateProjectPath; }
	bool HasNewProjectCreated() const { return m_projectCreated; }
	void ResetProjectCreatedFlag() { m_projectCreated = false; }

	void ToggleNewProjectModal();
	void GetNewProjectModalWindow();
};

#endif //EDITOR_WINDOWS_NEWPROJECTMODAL_H