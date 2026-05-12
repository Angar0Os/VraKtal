#pragma once
#include "./ImguiWindowBase.h"

class ImGuiWindows;
class RessourceManager;
class AssetManager;

class WindowInspector : public ImguiWindowBase
{
public:
	WindowInspector(ImGuiWindows& _windows , RessourceManager& _reManager , AssetManager& _astManager);
	~WindowInspector();

    void Draw() override;

private : 
    ImGuiWindows& m_windows;
    RessourceManager* m_ressourceManager;
    AssetManager& m_assetManager;
};