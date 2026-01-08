#pragma once

#include "../header/utils/fileTypeDetector.h"

#include <filesystem>

struct FileEntry {
	std::filesystem::path path;
	std::string filename;
	FileType fileType;
	bool isDirectory;
};

class ContentDrawer
{
private:
	std::filesystem::path m_currentPath;
	std::vector<FileEntry> m_cachedFiles;
	bool m_needsRefresh;

	const char* GetIconForFileType(FileType type);
	void RefreshFileList();

public:
	ContentDrawer();

	void GetContentDrawerWindow();
};