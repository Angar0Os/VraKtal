#pragma once

#include <windows/ImguiWindowBase.h>
#include <utils/NamedStorageMapped.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include <imgui/imgui.h>

class AssetManager;
class ImGuiWindows;
struct InterfaceStorage;

namespace graphics::assets
{
    struct Material;
    struct Mesh;
}

class WindowAsset : public ImguiWindowBase
{
public:
    WindowAsset(ImGuiWindows& _imGuiWindows);
    ~WindowAsset();

    void Draw() override;

private:
    using StorageDrawerFn = void(*)(WindowAsset&, InterfaceStorage*);
    using PropertyDrawerFn = void(*)(WindowAsset&, InterfaceStorage*, uint32_t);

private:
    template<typename TAsset>
    void RegisterAssetType(const std::string& _displayName);

    template<typename TAsset>
    static void DrawTypedStorage(WindowAsset& _window, InterfaceStorage* _storage);

    template<typename TAsset>
    static void DrawTypedProperties(WindowAsset& _window, InterfaceStorage* _storage, uint32_t _assetID);

    template<typename TAsset>
    void DrawStorageContent(Storage<TAsset>& _storage);

    template<typename TAsset>
    void DrawAssetProperties(TAsset& _asset);

private:
    void DrawToolbar();
    void DrawAssetList();
    void DrawSelectedAssetProperties();

    void DrawUnknownStorage(std::type_index _type, InterfaceStorage* _storage);

    void DrawAssetProperties(graphics::assets::Material& _material);
    void DrawAssetProperties(graphics::assets::Mesh& _mesh);

    bool DrawStringProperty(const char* _label, std::string& _value, size_t _bufferSize = 512);

    bool PassSearch(const std::string& _typeName, const std::string& _path, uint32_t _id) const;
    std::string GetTypeDisplayName(std::type_index _type) const;
    std::string GetReadableTypeName(std::type_index _type) const;

private:
    ImGuiWindows& m_windowManager;
    AssetManager& m_assetManager;

    std::unordered_map<std::type_index, StorageDrawerFn> m_storageDrawers;
    std::unordered_map<std::type_index, PropertyDrawerFn> m_propertyDrawers;
    std::unordered_map<std::type_index, std::string> m_typeDisplayNames;

    char m_searchBuffer[256] = {};

    std::type_index m_selectedType = typeid(void);
    uint32_t m_selectedAssetID = UINT32_MAX;
};

template<typename TAsset>
void WindowAsset::RegisterAssetType(const std::string& _displayName)
{
    const std::type_index type = std::type_index(typeid(TAsset));

    m_typeDisplayNames[type] = _displayName;
    m_storageDrawers[type] = &WindowAsset::DrawTypedStorage<TAsset>;
    m_propertyDrawers[type] = &WindowAsset::DrawTypedProperties<TAsset>;
}

template<typename TAsset>
void WindowAsset::DrawTypedStorage(WindowAsset& _window, InterfaceStorage* _storage)
{
    if (_storage == nullptr)
        return;

    Storage<TAsset>* typedStorage = static_cast<Storage<TAsset>*>(_storage);
    _window.DrawStorageContent<TAsset>(*typedStorage);
}

template<typename TAsset>
void WindowAsset::DrawTypedProperties(WindowAsset& _window, InterfaceStorage* _storage, uint32_t _assetID)
{
    if (_storage == nullptr)
        return;

    Storage<TAsset>* typedStorage = static_cast<Storage<TAsset>*>(_storage);

    bool exists = false;

    for (const auto& [path, id] : typedStorage->data.GetNameIdMap())
    {
        if (id == _assetID)
        {
            exists = true;
            break;
        }
    }

    if (!exists)
    {
        ImGui::TextDisabled("Selected asset no longer exists.");
        return;
    }

    TAsset& asset = typedStorage->data.Get(_assetID);
    _window.DrawAssetProperties(asset);
}

template<typename TAsset>
void WindowAsset::DrawStorageContent(Storage<TAsset>& _storage)
{
    const std::type_index type = std::type_index(typeid(TAsset));
    const std::string typeName = GetTypeDisplayName(type);

    std::vector<std::pair<std::string, uint32_t>> assets;

    for (const auto& [path, id] : _storage.data.GetNameIdMap())
    {
        if (PassSearch(typeName, path, id))
            assets.push_back({ path, id });
    }

    std::sort(
        assets.begin(),
        assets.end(),
        [](const auto& a, const auto& b)
        {
            return a.first < b.first;
        }
    );

    if (assets.empty())
    {
        ImGui::TextDisabled("Empty");
        return;
    }

    for (const auto& [path, id] : assets)
    {
        ImGui::PushID(static_cast<int>(id));

        const bool selected =
            m_selectedType == type &&
            m_selectedAssetID == id;

        const std::string label =
            "[" + std::to_string(id) + "] " + path;

        if (ImGui::Selectable(label.c_str(), selected))
        {
            m_selectedType = type;
            m_selectedAssetID = id;
        }

        if (ImGui::BeginPopupContextItem("AssetContextMenu"))
        {
            if (ImGui::MenuItem("Copy path"))
                ImGui::SetClipboardText(path.c_str());

            if (ImGui::MenuItem("Copy ID"))
            {
                const std::string idText = std::to_string(id);
                ImGui::SetClipboardText(idText.c_str());
            }

            ImGui::EndPopup();
        }

        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();

            ImGui::TextUnformatted(typeName.c_str());
            ImGui::Separator();
            ImGui::Text("ID: %u", id);
            ImGui::TextWrapped("%s", path.c_str());

            ImGui::EndTooltip();
        }

        ImGui::PopID();
    }
}

template<typename TAsset>
void WindowAsset::DrawAssetProperties(TAsset& _asset)
{
    ImGui::TextDisabled("No property drawer registered for this asset type.");
}