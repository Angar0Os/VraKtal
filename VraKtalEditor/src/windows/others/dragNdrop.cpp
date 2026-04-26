#include "../../../include/windows/others/dragNdrop.h"
#include "../../../include/contentDrawer.h"
#include "../../../include/imGuiWindows.h"

#include <scene/timeline/components/light.h>
#include <scene/timeline/components/mesh.h>

#include <graphics/resources/object/mesh.h>
#include <core/manager/ressourceManager.h>
#include <imgui/imgui_internal.h>

constexpr const char* PAYLOAD_FILE_ENTRY = "PAYLOAD_FILE_ENTRY";

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
            _meshID = m_reManager.GetRessourceID<graphics::resources::Mesh>(droppedEntry->GetRelativeFileLocation().string());
            return droppedEntry;
        }
    }
    return nullptr;
}

constexpr const char* PAYLOAD_MATERIAL_INDEX = "PAYLOAD_MATERIAL_INDEX";

template<>
void DragNDrop::Drag(MaterialIndex& _materialIndex)
{
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        ImGui::SetDragDropPayload(
            PAYLOAD_MATERIAL_INDEX,
            &_materialIndex,
            sizeof(MaterialIndex)
        );

        ImGui::Text("Material Index: %u", _materialIndex);

        ImGui::EndDragDropSource();
    }
}

template<>
MaterialIndex* DragNDrop::Content(MaterialIndex& _materialIndex)
{
    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(PAYLOAD_MATERIAL_INDEX))
    {
        IM_ASSERT(payload->DataSize == sizeof(MaterialIndex));

        const MaterialIndex* droppedMaterialIndex =
            static_cast<const MaterialIndex*>(payload->Data);

        _materialIndex = *droppedMaterialIndex;

        return &_materialIndex;
    }

    return nullptr;
}