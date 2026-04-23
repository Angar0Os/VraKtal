#include "../../include/windows/windowInspector.h"
#include "../../include/imGuiWindows.h"
#include "../../include/windows/others/rightClick.h"


#include <imgui/imgui.h>

#include <scene/scene.h>
#include <scene/timeline/entityBase.h>
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

        EntityID ID = m_windows.IsSelectedItemType<EntityID>() ? m_windows.GetSelectedItem<EntityID>() : INVALID_ENTITY;

        if (ID != INVALID_ENTITY)
        {
            std::string& label = scene->GetEntityComponent<std::string>(ID);
            ImGui::Text("%s", label.c_str());

            if (scene->GetComponentStorage<timeline::MeshInstance>().Has(ID))
            {
                m_windows.GetInspect()->Draw<timeline::MeshInstance>(scene->GetComponentStorage<timeline::MeshInstance>().Get(ID));
            }

            if (scene->GetComponentStorage<timeline::Light>().Has(ID))
            {
                m_windows.GetInspect()->Draw<timeline::Light>(scene->GetComponentStorage<timeline::Light>().Get(ID));
            }

            m_windows.GetRightClick()->Draw<WindowInspector>(this);
        }
        else
        {
            ImGui::Text("Nothing to see here");
        }
        
    }
    m_windows.EndWindow("Inspector");
}