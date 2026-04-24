#include "../../../include/windows/others/dragNdrop.h"
#include "../../../include/contentDrawer.h"
#include "../../../include/imGuiWindows.h"

#include <scene/timeline/components/light.h>
#include <scene/timeline/components/mesh.h>

#include <graphics/resources/object/mesh.h>
#include <core/manager/ressourceManager.h>

constexpr const char* PAYLOAD_FILE_ENTRY = "PAYLOAD_FILE_ENTRY";

template<>
void DragNDrop::Drag(FileEntry& _fileEntry) 
{
	if (ImGui::BeginDragDropSource())
	{
		FileEntry* payload = &_fileEntry;

		ImGui::SetDragDropPayload(
			PAYLOAD_FILE_ENTRY,
			&payload,
			sizeof(FileEntry*)
		);
		
		const char* icon = FileHelper::GetFileTypeIcon(_fileEntry.fileType);

		ImGui::TextUnformatted(icon);
		ImGui::EndDragDropSource();
	}
}

template<>
FileEntry* DragNDrop::Drop(FileEntry& _fileEntry)
{
	constexpr const char* PAYLOAD_FILE_ENTRY = "PAYLOAD_FILE_ENTRY";

	FileEntry* droppedEntry = nullptr;

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(PAYLOAD_FILE_ENTRY))
		{
			IM_ASSERT(payload->DataSize == sizeof(FileEntry*));

			droppedEntry = *static_cast<FileEntry**>(payload->Data);
		}

		ImGui::EndDragDropTarget();
	}

	return droppedEntry;
}

template<>
FileEntry* DragNDrop::Drop(Mesh_ID& _meshID)
{
	constexpr const char* PAYLOAD_FILE_ENTRY = "PAYLOAD_FILE_ENTRY";
	graphics::resources::Mesh* droppedMesh = nullptr;
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(PAYLOAD_FILE_ENTRY))
		{
			IM_ASSERT(payload->DataSize == sizeof(FileEntry*));
			FileEntry* droppedEntry = *static_cast<FileEntry**>(payload->Data);
			if (droppedEntry->fileType == FileType::MeshGLB || droppedEntry->fileType == FileType::MeshGLTF || droppedEntry->fileType == FileType::MeshOBJ)
			{
				_meshID = m_reManager.GetRessourceID<graphics::resources::Mesh>(droppedEntry->GetRelativeFileLocation().string());
                std::cout << "Mesh Dropped with ID: " << _meshID << " path: " << droppedEntry->GetRelativeFileLocation().string() << std::endl;
			}
			ImGui::EndDragDropTarget();
            return droppedEntry;
		}
		
		ImGui::EndDragDropTarget();
	}
	
	return nullptr;
}