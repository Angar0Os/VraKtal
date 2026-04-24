#include "../../include/windows/windowInspector.h"
#include "../../include/imGuiWindows.h"
#include "../../include/contentDrawer.h"

#include "../../include/windows/others/rightClick.h"
#include "../../include/windows/others/meshPlot.h"

#include "../../include/utils/fileTypeDetector.h"

#include <imgui/imgui.h>

#include <scene/scene.h>
#include <scene/timeline/entityBase.h>
#include <scene/timeline/components/mesh.h>
#include <scene/timeline/components/light.h>

#include <core/manager/ressourceManager.h>

WindowInspector::WindowInspector(ImGuiWindows& _windows, RessourceManager& _reManager) : m_windows(_windows), m_ressourceManager(&_reManager)
{
}

WindowInspector::~WindowInspector()
{
}

void WindowInspector::Draw()
{
    if (m_windows.BeginWindow("Inspector"))
    {
        if (m_windows.IsSelectedItemType<std::monostate>())
        {
            ImGui::Text("Nothing to see here");
        }
        else if (m_windows.IsSelectedItemType<EntityID>())
        {
            Scene* scene = m_windows.GetScene();

            EntityID ID = m_windows.GetSelectedItem<EntityID>();

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
        }
        else if (m_windows.IsSelectedItemType<FileEntry*>())
        {
            FileEntry* file = m_windows.GetSelectedItem<FileEntry*>();
            if (file->fileType == FileType::MeshGLTF || file->fileType == FileType::MeshOBJ)
            {
                ImGui::Text("Mesh Preview: %s" , file->path);
                uint32_t ressourceID = m_ressourceManager->GetRessourceID<graphics::resources::Mesh>(file->GetRelativeFileLocation().string());
                if (ressourceID != 0xFFFFFFFFu)
                {
                    graphics::resources::Mesh& _mesh = m_ressourceManager->GetRessource<graphics::resources::Mesh>(ressourceID);
                    m_windows.GetMeshPlot()->Draw(&_mesh);
                }
            }
            else
            {
                ImGui::Text("Unsuported Object");
            }
        }
    }
    m_windows.EndWindow("Inspector");
}