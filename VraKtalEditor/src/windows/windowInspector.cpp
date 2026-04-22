#include "../../include/windows/windowInspector.h"
#include "../../include/imGuiWindows.h"

#include <scene/scene.h>

#include <imgui/imgui.h>

#include <scene/timeline/components/mesh.h>
#include <scene/timeline/components/light.h>

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
        Scene* scene = m_windows.GetScene();
        EntityID& ID = m_windows.selectedItem;

        if (scene->GetComponentStorage<timeline::MeshInstance>().Has(ID))
        {
            m_windows.GetInspect()->Draw<timeline::MeshInstance>(scene->GetComponentStorage<timeline::MeshInstance>().Get(ID));
        }

        if (scene->GetComponentStorage<timeline::Light>().Has(ID))
        {
            m_windows.GetInspect()->Draw<timeline::Light>(scene->GetComponentStorage<timeline::Light>().Get(ID));
        }
        
        m_windows.EndWindow("Inspector");
    }
}