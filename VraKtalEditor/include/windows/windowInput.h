#pragma once
#include "ImguiWindowBase.h"
#include <core/input/keys.h>

#include <vector>

namespace core {
	class Input;
}

struct KeyActions;
struct Axis2DAction;
struct ImVec2;
class ImGuiWindows;

class WindowInput : public ImguiWindowBase
{
public:
	WindowInput(core::Input& _input , ImGuiWindows* _imGuiWindow);
	~WindowInput();

	void Draw() override;

private:
	core::Input& m_input;

	bool bIsEditingInput = false;

	KeyActions* m_keyActionEditing;
	std::vector<input::Key> m_keysRecoreded;
	bool bStartedInput = false;


	Axis2DAction* m_axisActionEditing;
	int m_openedAxisAction = -1;
	int AxisToChange = -1;


private : //style 
	bool InputButton(const char* label, const ImVec2& size);

    ImGuiWindows* m_imGuiWindow;
};