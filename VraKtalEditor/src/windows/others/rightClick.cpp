#include "../../../include/windows/others/rightClick.h"
#include "../../../include/imGuiWindows.h"

#include <scene/scene.h>


#include <imgui/imgui.h>


RightClick::RightClick(ImGuiWindows* _windows) : m_windows(_windows)
{
    
}

RightClick::~RightClick()
{
}

template<>
void RightClick::Content(WindowHierarchy* _window)
{
    if (ImGui::Button("Add Entity"))
    {
        m_windows->GetScene()->CreateEntity();
        CloseMenu();
    }
}

template<>
void RightClick::Content(EntityID* _ID) 
{
    if (ImGui::Button("Duplicate Entity"))
    {
        Scene* sc = m_windows->GetScene();

    }

    if (ImGui::Button("Destroy Entity"))
    {
        m_windows->GetScene()->DestroyEntity(*_ID);
    }
}