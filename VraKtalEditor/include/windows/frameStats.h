#pragma once
#include "ImguiWindowBase.h"

class ImGuiWindows;
namespace core{
	class Time;
}

class WindowStat : public ImguiWindowBase
{
public:
	WindowStat(ImGuiWindows& _imguiWindows);
	~WindowStat();

	void Draw() override;

private:
	ImGuiWindows& m_imguiWindow;
	const core::Time& m_time;
};