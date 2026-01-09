#pragma once

#include "../header/utils/fileTypeDetector.h"

#include <filesystem>
#include <set>

struct FileEntry {
	std::filesystem::path path;
	std::string filename;
	FileType fileType;
	bool isDirectory;
	bool isSelected;
};

enum class ClipboardAction {
	None,
	Copy,
	Cut
};

class ContentDrawer
{
private:
	std::filesystem::path m_currentPath;
	std::vector<FileEntry> m_cachedFiles;
	bool m_needsRefresh;
	std::set<size_t> m_selectedIndices;

	bool m_showRenameDialog = false;
	bool m_showDeleteDialog = false;
	char m_renameBuffer[256] = "";
	size_t m_renameTargetIndex = 0;

	ClipboardAction m_clipboardAction = ClipboardAction::None;
	std::vector<std::filesystem::path> m_clipboardPaths;

	const char* GetIconForFileType(FileType type);
	void RefreshFileList();
	void ClearSelection();
	void HandleFileActions();

	void PerformCopy();
	void PerformCut();
	void PerformDelete();
	void PerformPaste();
	void PerformRename();
	void ShowDeleteDialog();
	void ShowRenameDialog();

	void PerformImport();

public:
	ContentDrawer();

	void GetContentDrawerWindow();
};