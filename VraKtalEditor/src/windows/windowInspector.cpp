#include "../../include/windows/ImguiWindowBase.h"
#include "../../include/windows/others/inspector.h"
#include "../../include/imGuiWindows.h"
#include "../../include/windows/windowInspector.h"

#include <core/gpu/buffer.h>

WindowInspector::WindowInspector(ImGuiWindows& _windows) : m_windows(_windows)
{
}

WindowInspector::~WindowInspector()
{
}

void WindowInspector::Draw()
{
    if (m_windows.BeginWindow("Inspector"))
    {


        m_windows.EndWindow("Inspector");
    }
}

