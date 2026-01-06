#pragma once

#include <filesystem>

class ContentDrawer
{
private:
	std::filesystem::path m_currentPath;

public:
	ContentDrawer();

	void GetContentDrawerWindow();
};