#ifndef EDITOR_WINDOWS_CONTENTDRAWER_H
#define EDITOR_WINDOWS_CONTENTDRAWER_H
#pragma once

#include "utils/fileTypeDetector.h"
#include "command/commandHistory.h"

#include <filesystem>
#include <set>
#include <memory>

struct FileEntry {
	std::filesystem::path path;
	std::string filename;
	FileType fileType;
	bool isDirectory;
	bool isSelected;
	
	std::filesystem::path GetRelativeFileLocation();
};

class ImGuiWindows;


enum class ClipboardAction {
	None,
	Copy,
	Cut
};

struct FileHelper
{
	static const char* GetFileTypeIcon(FileType type);
};

class ContentDrawer
{
private:
	std::filesystem::path m_currentPath;
	std::filesystem::path m_baseAssetPath;
	std::vector<FileEntry> m_cachedFiles;
	bool m_needsRefresh;
	std::set<size_t> m_selectedIndices;

	bool m_showRenameDialog = false;
	bool m_showDeleteDialog = false;
	char m_renameBuffer[256] = "";
	size_t m_renameTargetIndex = 0;

	ClipboardAction m_clipboardAction = ClipboardAction::None;
	std::vector<std::filesystem::path> m_clipboardPaths;

	command::CommandHistory* m_commandHistory = nullptr;

	void RefreshFileList();
	void ClearSelection();
	void HandleFileActions();

	void PerformCopy();
	void PerformCreateFolder();
	void PerformCut();
	void PerformDelete();
	void PerformPaste();
	void PerformRename();
	void ShowDeleteDialog();
	void ShowRenameDialog();

	void PerformImport();
	const char* GetIconForFileType(FileType type);

public:
	ContentDrawer(ImGuiWindows* _windows);
	~ContentDrawer();

	void GetContentDrawerWindow();
	void SetCurrentPath(std::filesystem::path newPath);
	void SetCommandHistory(command::CommandHistory* history);

	void HandleExternalFileDrop(const std::vector<std::string>& filePaths);

private:
    ImGuiWindows* m_windowManager;
};

#endif //EDITOR_WINDOWS_CONTENTDRAWER_H
