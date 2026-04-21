#pragma once
#include "./ImguiWindowBase.h"

class ImGuiWindows;

class WindowInspector : public ImguiWindowBase
{
public:
	WindowInspector(ImGuiWindows& _windows);
	~WindowInspector();

    void Draw() override;

private : 
    ImGuiWindows& m_windows;
};