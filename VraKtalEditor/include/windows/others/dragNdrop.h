#pragma once
#include <core/manager/ressourceManager.h>
#include <core/manager/assetManager.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <scene/timeline/entityBase.h>
#include "folder.h"

namespace graphics::resources
{
    struct Mesh;
}
namespace graphics::assets
{
    struct Mesh;
}


namespace scene::timeline::components
{
    class Light;
    struct Mesh;
}

struct FileEntry;
class ImGuiWindows;
class RessourceManager;
class AssetManager;

namespace hierarchy {
    struct Folder;
}

using Mesh_ID = uint32_t;
static constexpr Mesh_ID INVALID_ID = 0xFFFFFFFFu;

template<typename T, typename Tag>
struct PayloadWrap
{
    T value{};

    PayloadWrap() = default;
    PayloadWrap(T v) : value(v) {}

    operator T() const
    {
        return value;
    }

    PayloadWrap& operator=(T v)
    {
        value = v;
        return *this;
    }
};
template<typename TAsset>
using AssetPayload = PayloadWrap<uint32_t, TAsset>;
template<typename TAsset>
struct AssetPayloadTraits;
template<>
struct AssetPayloadTraits<graphics::assets::Mesh>
{
    static constexpr const char* Name = "PAYLOAD_ASSET_MESH";
    static constexpr const char* DisplayName = "Mesh";
};
template<>
struct AssetPayloadTraits<graphics::assets::Material>
{
    static constexpr const char* Name = "PAYLOAD_ASSET_MATERIAL";
    static constexpr const char* DisplayName = "Material";
};

class Vraktal;

struct DragNDrop
{
    DragNDrop(ImGuiWindows& _windwManager, Vraktal& _vraktal);

    template<typename T>
    void Drag(T& object) {
        if (ImGui::BeginPopup("DragAvailability"))
        {
            ImGui::Text("No DragAvailable");
            ImGui::EndPopup();
        };
    };

    template<typename TAsset>
    void Drag(AssetPayload<TAsset>& _payload);
    
    template<typename DraggedType , typename DroppedReceived>
    DraggedType* DropWindow(DroppedReceived& object) {

        DraggedType* toReturn = nullptr;
		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
		{
			ImVec2 var = ImGui::GetWindowPos();
			var.x = ImGui::GetWindowSize().x + var.x;
			var.y = ImGui::GetWindowSize().y + var.y;

			ImRect rect(ImGui::GetWindowPos(), var);

            if (ImGui::BeginDragDropTargetCustom(rect, ImGui::GetCurrentWindow()->ID))
			{
                toReturn = Content<DraggedType , DroppedReceived>(object);
                ImGui::EndDragDropTarget();
			}
		}
		return toReturn;
    }

    template<typename DraggedType, typename DroppedReceived>
    DraggedType* DropItem(DroppedReceived& object) {
        DraggedType* droppedEntry = nullptr;
        if (ImGui::BeginDragDropTarget())
        {
            droppedEntry = Content<DraggedType, DroppedReceived>(object);
            ImGui::EndDragDropTarget();
        }
        return droppedEntry;
    }

    template<typename DraggedType>
    DraggedType* DropItem() {
        DraggedType* droppedEntry = nullptr;
        if (ImGui::BeginDragDropTarget())
        {
            droppedEntry = Content<DraggedType>();
            ImGui::EndDragDropTarget();
        }
        return droppedEntry;
    }

private:
    ImGuiWindows& m_windowManager;
    RessourceManager& m_reManager;
    AssetManager& m_astManager;


    template<typename DraggedType, typename DroppedReceived>
    DraggedType* Content(DroppedReceived& object) {
        ImGui::BeginPopup("DropAvailability");
        ImGui::Text("No DropAvailable");
        ImGui::EndPopup();
        return nullptr;
    }

    template<typename DraggedType>
    DraggedType* Content() {
        ImGui::BeginPopup("DropAvailability");
        ImGui::Text("No DropAvailable");
        ImGui::EndPopup();
        return nullptr;
    }

    template<typename TAsset>
    TAsset* AcceptAssetContent(uint32_t& _assetID)
    {
        using Payload = AssetPayload<TAsset>;

        constexpr const char* payloadName = AssetPayloadTraits<TAsset>::Name;

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadName))
        {
            IM_ASSERT(payload->DataSize == sizeof(Payload));

            const Payload* assetPayload =
                static_cast<const Payload*>(payload->Data);

            _assetID = assetPayload->value;

            return &m_astManager.GetAsset<TAsset>(_assetID);
        }

        return nullptr;
    };

};

template<>
void DragNDrop::Drag(FileEntry& _fileEntry);
template<>
FileEntry* DragNDrop::Content(FileEntry& _fileEntry);
#pragma region Inspector

struct MaterialIndexTag {};
using MaterialIndexPayload = PayloadWrap<uint32_t, MaterialIndexTag>;
using MaterialIndex = uint32_t;
template<>
void DragNDrop::Drag(MaterialIndexPayload& _materialIndex);
template<>
MaterialIndexPayload* DragNDrop::Content(MaterialIndex& _meshID);
template<>
FileEntry* DragNDrop::Content(Mesh_ID& _meshID);
#pragma endregion

struct FolderIDTag {};
using FolderIDPayload = PayloadWrap<uint32_t, FolderIDTag>;
template<>
void DragNDrop::Drag(hierarchy::Folder& _folder);
template<>
hierarchy::Folder* DragNDrop::Content(hierarchy::FolderID& _folder);

struct EntityPayload { //Fun fact imgui peut pas prendre de std::vector 
    uint32_t count = 0;
    EntityID entities[128];
};
template<>
void DragNDrop::Drag(EntityPayload& _payload);
template<>
EntityPayload* DragNDrop::Content();

template<typename TAsset>
inline void DragNDrop::Drag(AssetPayload<TAsset>& _payload)
{
    const char* payload_name = AssetPayloadTraits<TAsset>::Name;
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        ImGui::SetDragDropPayload(
            payload_name,
            &_payload,
            sizeof(AssetPayload<TAsset>)
        );
        ImGui::Text("%s %s", AssetPayloadTraits<TAsset>::DisplayName, m_astManager.GetAsset<TAsset>(_payload.value).name.c_str());
        ImGui::EndDragDropSource();
    }
};

template<>
graphics::assets::Mesh* DragNDrop::Content<graphics::assets::Mesh, Mesh_ID>(Mesh_ID& _meshID);

template<>
graphics::assets::Material* DragNDrop::Content<graphics::assets::Material, uint32_t>(uint32_t& _assetID);