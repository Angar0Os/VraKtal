#include "../../../include/windows/others/dragNdrop.h"
#include "../../../include/windows/others/folder.h"
#include "../../../include/contentDrawer.h"
#include "../../../include/imGuiWindows.h"

#include <scene/timeline/components/light.h>
#include <scene/timeline/components/mesh.h>

#include <graphics/resources/object/mesh.h>

#include <core/manager/ressourceManager.h>
#include <core/manager/assetManager.h>
#include <factory/meshFactory.h>
#include <factory/materialFactory.h>

#include <vraktal.h>
#include <scene/scene.h>
#include <imgui/imgui_internal.h>



constexpr const char* PAYLOAD_FILE_ENTRY = "PAYLOAD_FILE_ENTRY";

DragNDrop::DragNDrop(ImGuiWindows& _windwManager, Vraktal& _vraktal) : m_windowManager(_windwManager), m_reManager(_vraktal.GetRessourceManager()) , m_astManager(_vraktal.GetAssetManager()){}

template<>
void DragNDrop::Drag(FileEntry& _fileEntry) 
{
	if (ImGui::BeginDragDropSource())
	{
		FileEntry* payload = &_fileEntry;
		ImGui::SetDragDropPayload(PAYLOAD_FILE_ENTRY,&payload,sizeof(FileEntry*));
		const char* icon = FileHelper::GetFileTypeIcon(_fileEntry.fileType);
		ImGui::TextUnformatted(icon);
		ImGui::EndDragDropSource();
	}
}

template<>
FileEntry* DragNDrop::Content(FileEntry& _fileEntry)
{
	FileEntry* droppedEntry = nullptr;
	if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("PAYLOAD_FILE_ENTRY"))
	{
		IM_ASSERT(payload->DataSize == sizeof(FileEntry*));
		droppedEntry = *static_cast<FileEntry**>(payload->Data);
	}
	return droppedEntry;
}

template<>
FileEntry* DragNDrop::Content(Mesh_ID& _meshID)
{
    constexpr const char* PAYLOAD_FILE_ENTRY = "PAYLOAD_FILE_ENTRY";
    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(PAYLOAD_FILE_ENTRY))
    {
        IM_ASSERT(payload->DataSize == sizeof(FileEntry*));
        FileEntry* droppedEntry = *static_cast<FileEntry**>(payload->Data);
        if (!droppedEntry)
            return nullptr;
        if (droppedEntry->fileType == FileType::MeshGLB || droppedEntry->fileType == FileType::MeshGLTF || droppedEntry->fileType == FileType::MeshOBJ)
        {
            /*
                DRAG FILEENTRTY OF TYPE MESH
            */
            return droppedEntry;
        }
    }
    return nullptr;
}

template<>
void DragNDrop::Drag(graphics::assets::Mesh& _meshAsset) 
{
    const char* PAYLOAD_ASSET_MESH = "PAYLOAD_ASSET_MESH";
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        ImGui::SetDragDropPayload(
            PAYLOAD_ASSET_MESH,
            &_meshAsset,
            sizeof(graphics::assets::Mesh)
        );
        ImGui::Text("Material Index: %u", _meshAsset);
        ImGui::EndDragDropSource();
    }
}

template<>
void DragNDrop::Drag(graphics::assets::Material& _material) 
{
    const char* PAYLOAD_ASSET_MATERIAL = "PAYLOAD_ASSET_MATERIAL";
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        ImGui::SetDragDropPayload(
            PAYLOAD_ASSET_MATERIAL,
            &_material,
            sizeof(graphics::assets::Material)
        );
        ImGui::Text("Material Index: %u", _material.name);
        ImGui::EndDragDropSource();
    }
}

template<>
graphics::assets::Mesh* DragNDrop::Content(Mesh_ID& _meshID)
{
    Mesh_ID meshAssetID = INVALID_ID;

    graphics::assets::Mesh* meshAsset = AcceptAssetContent<graphics::assets::Mesh>(meshAssetID);

    if (!meshAsset)
        return nullptr;

    if (!m_reManager.HasResource<graphics::resources::Mesh>(meshAssetID))
    {
        _meshID = m_reManager.CreateRessource<graphics::resources::Mesh>(*meshAsset);
    }
    else
    {
        _meshID = meshAssetID;
    }

    return meshAsset;
}

template<>
graphics::assets::Material* DragNDrop::Content(uint32_t& _assetID)
{
    uint32_t AssetID = INVALID_ID;

    graphics::assets::Material* materialAsset = AcceptAssetContent<graphics::assets::Material>(AssetID);

    if (!materialAsset)
        return nullptr;

    if (!m_reManager.HasResource<graphics::resources::Material>(AssetID))
    {
        _assetID = m_reManager.CreateRessource<graphics::resources::Material>(*materialAsset);
    }
    else
    {
        _assetID = AssetID;
    }

    return materialAsset;
}


template<>
void DragNDrop::Drag(MaterialIndexPayload& _materialIndex)
{
    const char* PAYLOAD_MATERIAL_INDEX = "PAYLOAD_MATERIAL_INDEX";
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        ImGui::SetDragDropPayload(
            PAYLOAD_MATERIAL_INDEX,
            &_materialIndex,
            sizeof(MaterialIndexPayload)
        );

        ImGui::Text("Material Index: %u", _materialIndex);

        ImGui::EndDragDropSource();
    }
}

template<>
MaterialIndexPayload* DragNDrop::Content(MaterialIndex& _materialIndex)
{
    const char* PAYLOAD_MATERIAL_INDEX = "PAYLOAD_MATERIAL_INDEX";
    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(PAYLOAD_MATERIAL_INDEX))
    {
        IM_ASSERT(payload->DataSize == sizeof(MaterialIndexPayload));

        MaterialIndexPayload* droppedMaterialPayload =
            static_cast<MaterialIndexPayload*>(payload->Data);

        _materialIndex = *droppedMaterialPayload;

        return droppedMaterialPayload;
    }

    return nullptr;
}

template<>
void DragNDrop::Drag(EntityPayload& _payload)
{
    if (ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("HIERARCHY_ENTITY", &_payload, sizeof(EntityPayload));
        if (_payload.count > 1)
        {
            ImGui::Text("Moving %d Entities", _payload.count);
        }
        else if (_payload.count == 1)
        {
            EntityID entity = _payload.entities[0];

            if (m_windowManager.GetScene()->GetComponentStorage<std::string>().Has(entity))
            {
                const std::string& name =
                    m_windowManager.GetScene()->GetComponentStorage<std::string>().Get(entity);

                ImGui::Text("Moving %s", name.c_str());
            }
        }
        else
        {
            ImGui::Text("ENTITIES SHOULD HAVE A NAME");
        }
        ImGui::EndDragDropSource();
    }
}

template<>
EntityPayload* DragNDrop::Content()
{
    static EntityPayload lastPayload;

    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ENTITY"))
    {
        if (payload->DataSize != sizeof(EntityPayload))
            return nullptr;
        lastPayload = *static_cast<const EntityPayload*>(payload->Data);
        return &lastPayload;
    }

    return nullptr;
}

template<>
void DragNDrop::Drag(hierarchy::Folder& _folder)
{
    if (ImGui::BeginDragDropSource())
    {
        hierarchy::Folder* folderPtr = &_folder;
        ImGui::SetDragDropPayload(
            "HIERARCHY_FOLDER",
            &folderPtr,
            sizeof(hierarchy::Folder*)
        );
        std::string Name = _folder.name;
        ImGui::TextUnformatted(Name.c_str());
        ImGui::EndDragDropSource();
    }
}

template<>
hierarchy::Folder* DragNDrop::Content(hierarchy::FolderID& _folder)
{
    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_FOLDER"))
    {
        IM_ASSERT(payload->DataSize == sizeof(hierarchy::Folder*));
        const hierarchy::Folder* const* payloadPtr =
            static_cast<const hierarchy::Folder* const*>(payload->Data);

        hierarchy::Folder* droppedFolder = const_cast<hierarchy::Folder*>(*payloadPtr);
        _folder = droppedFolder->id;
        return droppedFolder;
    }
    return nullptr;
}

