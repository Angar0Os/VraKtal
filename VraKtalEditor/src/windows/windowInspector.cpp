#include "../../include/windows/windowInspector.h"
#include "../../include/imGuiWindows.h"
#include "../../include/contentDrawer.h"

#include "../../include/windows/others/meshPlot.h"
#include "../../include/windows/others/inspector.h"
#include "../../include/windows/others/dragNdrop.h"
#include "../../include/windows/popup/rightClick.h"

#include "../../include/utils/fileTypeDetector.h"

#include <imgui/imgui.h>

#include <scene/scene.h>
#include <scene/timeline/entityBase.h>
#include <scene/timeline/components/mesh.h>
#include <scene/timeline/components/light.h>

#include <core/manager/ressourceManager.h>
#include <core/manager/assetManager.h>
#include <graphics/resources/object/mesh.h>

WindowInspector::WindowInspector(ImGuiWindows& _windows, RessourceManager& _reManager, AssetManager& _astManager) : m_windows(_windows), m_ressourceManager(&_reManager) , m_assetManager(_astManager)
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
                //Handle Drop
                Mesh_ID draggedMeshID = INVALID_ID;
                m_windows.GetDragNDrop()->DropWindow<FileEntry, Mesh_ID>(draggedMeshID);
                if (draggedMeshID != INVALID_ID)
                {
                    scene->GetComponentStorage<timeline::MeshInstance>().Get(ID).assetID = draggedMeshID;
                }

                if (scene->GetComponentStorage<std::string>().Has(ID))
                {
                    std::string& label = scene->GetEntityComponent<std::string>(ID);
                    ImGui::Text("%s", label.c_str());
                    ImGui::SameLine();
                    ImGui::Text("#%d", ID);
                }
                else
                {
                    ImGui::Text("This Entity Have no name this is not normal behaviour");
                }

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
                uint32_t assetID = m_assetManager.GetAssetID<graphics::resources::Mesh>(file->GetRelativeFileLocation().string());
                if (assetID != INVALID_ID)
                {
                    ImGui::Text("Mesh Preview: %s" , file->path.string().c_str());
                    m_windows.GetMeshPlot()->Draw(&m_assetManager.GetAsset<graphics::assets::Mesh>(assetID));
                }
                else
                {
                    ImGui::Text("Mesh isn't loaded");
                    //Need to create asset from mesh
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