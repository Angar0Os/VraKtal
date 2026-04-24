#pragma once
#include "./ImguiWindowBase.h"

class ImGuiWindows;
class RessourceManager;

class WindowInspector : public ImguiWindowBase
{
public:
	WindowInspector(ImGuiWindows& _windows , RessourceManager& _reManager);
	~WindowInspector();

    void Draw() override;

private : 
    ImGuiWindows& m_windows;
    RessourceManager* m_ressourceManager;
};