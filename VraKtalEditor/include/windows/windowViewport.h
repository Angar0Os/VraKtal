#pragma once
#include "ImguiWindowBase.h"

class ImGuiWindows;

class WindowViewport : ImguiWindowBase
{
public:
	WindowViewport(ImGuiWindows& _imguiWindows);
	~WindowViewport();

	void Draw() override;

private:
	ImGuiWindows* m_imguiWindows;
};