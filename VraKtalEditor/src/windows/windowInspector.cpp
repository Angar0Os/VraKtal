#include "../../include/windows/ImguiWindowBase.h"
#include "../../include/windows/others/inspector.h"
#include "../../include/imGuiWindows.h"
#include "../../include/windows/windowInspector.h"

#include <core/gpu/buffer.h>
#include <scene/scene.h>

#include <imgui/imgui.h>
#include "../../include/windows/others/inspector.h"

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
            Scene* scene = m_windows.GetScene();
            EntityID& ID = m_windows.selectedItem;

            if (scene->GetComponentStorage<timeline::MeshInstance>().Has(ID))
            {
                glm::mat4& transform = scene->GetComponentStorage<timeline::MeshInstance>().Get(ID).temp_transform;
                Inspect::Draw(transform);
            }


        }
        m_windows.EndWindow("Inspector");
    }
}