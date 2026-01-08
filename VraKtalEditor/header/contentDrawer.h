#pragma once

#include "../header/utils/fileTypeDetector.h"

#include <filesystem>

class ContentDrawer
{
private:
	std::filesystem::path m_currentPath;
	const char* GetIconForFileType(FileType type);

public:
	ContentDrawer();

	void GetContentDrawerWindow();
};