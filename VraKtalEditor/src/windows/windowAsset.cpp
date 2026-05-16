#include <windows/windowAsset.h>

#include <imGuiWindows.h>
#include <vraktal.h>
#include <core/manager/assetManager.h>

#include <graphics/assets/material.h>
#include <graphics/assets/mesh.h>

#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <string>
#include <vector>

namespace
{
    std::string ToLower(std::string _value)
    {
        std::transform(
            _value.begin(),
            _value.end(),
            _value.begin(),
            [](unsigned char c)
            {
                return static_cast<char>(std::tolower(c));
            }
        );

        return _value;
    }

    void ReplaceAll(std::string& _value, const std::string& _from, const std::string& _to)
    {
        if (_from.empty())
            return;

        size_t startPos = 0;

        while ((startPos = _value.find(_from, startPos)) != std::string::npos)
        {
            _value.replace(startPos, _from.length(), _to);
            startPos += _to.length();
        }
    }

    std::string StripCppTypeName(std::string _name)
    {
        ReplaceAll(_name, "struct ", "");
        ReplaceAll(_name, "class ", "");

        const size_t lastNamespace = _name.rfind("::");

        if (lastNamespace != std::string::npos)
            return _name.substr(lastNamespace + 2);

        return _name;
    }
}

WindowAsset::WindowAsset(ImGuiWindows& _imGuiWindows)
    : m_windowManager(_imGuiWindows)
    , m_assetManager(_imGuiWindows.GetVraktal().GetAssetManager())
{
    RegisterAssetType<graphics::assets::Material>("Material");
    RegisterAssetType<graphics::assets::Mesh>("Mesh");
}

WindowAsset::~WindowAsset()
{
}

void WindowAsset::Draw()
{
    if (m_windowManager.BeginWindow("Assets", true))
    {
        DrawToolbar();

        ImGui::Separator();

        const float availableWidth = ImGui::GetContentRegionAvail().x;
        const float leftPanelWidth = availableWidth * 0.45f;

        ImGui::BeginChild("AssetListPanel", ImVec2(leftPanelWidth, 0.0f), true);
        DrawAssetList();
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("AssetPropertiesPanel", ImVec2(0.0f, 0.0f), true);
        DrawSelectedAssetProperties();
        ImGui::EndChild();
    }

    m_windowManager.EndWindow("Assets");
}

void WindowAsset::DrawToolbar()
{
    ImGui::SetNextItemWidth(-1.0f);

    ImGui::InputTextWithHint(
        "##AssetSearch",
        "Search assets...",
        m_searchBuffer,
        sizeof(m_searchBuffer)
    );
}

void WindowAsset::DrawAssetList()
{
    auto& storages = m_assetManager.GetAssetStorages();

    if (storages.empty())
    {
        ImGui::TextDisabled("No asset storage registered.");
        return;
    }

    std::vector<std::pair<std::type_index, InterfaceStorage*>> sortedStorages;

    for (auto& [type, storage] : storages)
    {
        if (storage != nullptr)
            sortedStorages.push_back({ type, storage });
    }

    std::sort(
        sortedStorages.begin(),
        sortedStorages.end(),
        [&](const auto& a, const auto& b)
        {
            return GetTypeDisplayName(a.first) < GetTypeDisplayName(b.first);
        }
    );

    for (auto& [type, storage] : sortedStorages)
    {
        const std::string typeName = GetTypeDisplayName(type);

        ImGui::PushID(storage);

        const bool opened = ImGui::TreeNodeEx(
            typeName.c_str(),
            ImGuiTreeNodeFlags_SpanAvailWidth |
            ImGuiTreeNodeFlags_DefaultOpen
        );

        if (opened)
        {
            auto drawerIt = m_storageDrawers.find(type);

            if (drawerIt != m_storageDrawers.end())
                drawerIt->second(*this, storage);
            else
                DrawUnknownStorage(type, storage);

            ImGui::TreePop();
        }

        ImGui::PopID();
    }
}

void WindowAsset::DrawSelectedAssetProperties()
{
    if (m_selectedType == std::type_index(typeid(void)) || m_selectedAssetID == UINT32_MAX)
    {
        ImGui::TextDisabled("Select an asset to edit its properties.");
        return;
    }

    auto& storages = m_assetManager.GetAssetStorages();

    auto storageIt = storages.find(m_selectedType);

    if (storageIt == storages.end() || storageIt->second == nullptr)
    {
        ImGui::TextDisabled("Selected asset storage is missing.");
        return;
    }

    const std::string typeName = GetTypeDisplayName(m_selectedType);

    ImGui::TextUnformatted(typeName.c_str());
    ImGui::Text("ID: %u", m_selectedAssetID);

    ImGui::Separator();

    auto propertyDrawerIt = m_propertyDrawers.find(m_selectedType);

    if (propertyDrawerIt == m_propertyDrawers.end())
    {
        DrawUnknownStorage(m_selectedType, storageIt->second);
        return;
    }

    propertyDrawerIt->second(*this, storageIt->second, m_selectedAssetID);
}

void WindowAsset::DrawUnknownStorage(std::type_index _type, InterfaceStorage* _storage)
{
    ImGui::TextDisabled("No drawer registered for this asset type.");
    ImGui::Text("RTTI type: %s", _type.name());
    ImGui::Text("Storage pointer: %p", _storage);
}

void WindowAsset::DrawAssetProperties(graphics::assets::Material& _material)
{
    DrawStringProperty("Name", _material.name);

    ImGui::SeparatorText("Textures");

    DrawStringProperty("Albedo", _material.albedoTexture);
    DrawStringProperty("Normal", _material.normalTexture);
    DrawStringProperty("Metallic", _material.metallicTexture);
    DrawStringProperty("Roughness", _material.roughnessTexture);
    DrawStringProperty("AO", _material.aoTexture);
    DrawStringProperty("Emissive", _material.emissiveTexture);

    if (ImGui::Button("Clear all textures"))
        _material.ClearAllTextures();

    ImGui::SeparatorText("Material Type");

    const char* materialTypeNames[] =
    {
        "PBR",
        "Unlit",
        "Skybox"
    };

    int currentType = static_cast<int>(_material.materialType);

    if (ImGui::Combo("Type", &currentType, materialTypeNames, IM_ARRAYSIZE(materialTypeNames)))
        _material.materialType = static_cast<graphics::MaterialType>(currentType);

    ImGui::SeparatorText("Surface");

    ImGui::ColorEdit3("Albedo Color", glm::value_ptr(_material.albedo));

    ImGui::DragFloat("Metallic", &_material.metallic, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("Roughness", &_material.roughness, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("AO", &_material.ao, 0.01f, 0.0f, 1.0f);

    ImGui::SeparatorText("Emission");

    ImGui::ColorEdit3("Emissive Color", glm::value_ptr(_material.emissive));
    ImGui::DragFloat("Emissive Strength", &_material.emissiveStrength, 0.01f, 0.0f, 100.0f);

    ImGui::SeparatorText("Rendering");

    ImGui::DragFloat("Opacity", &_material.opacity, 0.01f, 0.0f, 1.0f);
    ImGui::Checkbox("Double Sided", &_material.doubleSided);
}

void WindowAsset::DrawAssetProperties(graphics::assets::Mesh& _mesh)
{
    DrawStringProperty("Name", _mesh.name);
    DrawStringProperty("Path", _mesh.path);

    ImGui::SeparatorText("Statistics");

    ImGui::Text("Vertices: %u", _mesh.GetVertexCount());
    ImGui::Text("Indices: %u", _mesh.GetIndexCount());
    ImGui::Text("Triangles: %u", _mesh.GetIndexCount() / 3);
    ImGui::Text("Submeshes: %zu", _mesh.subMeshes.size());

    if (ImGui::Button("Recalculate Normals"))
        _mesh.RecalculateNormals();

    ImGui::SeparatorText("Submeshes");

    if (_mesh.subMeshes.empty())
    {
        ImGui::TextDisabled("No submeshes.");
        return;
    }

    for (size_t i = 0; i < _mesh.subMeshes.size(); ++i)
    {
        graphics::SubMesh& subMesh = _mesh.subMeshes[i];

        std::string label = subMesh.name.empty()
            ? "SubMesh " + std::to_string(i)
            : subMesh.name;

        ImGui::PushID(static_cast<int>(i));

        if (ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
        {
            DrawStringProperty("Name", subMesh.name);

            ImGui::Text("First Index: %u", subMesh.firstIndex);
            ImGui::Text("Index Count: %u", subMesh.indexCount);
            ImGui::Text("Vertex Offset: %u", subMesh.vertexOffset);
            ImGui::Text("Triangle Count: %u", subMesh.GetTriangleCount());

            int materialIndex = static_cast<int>(subMesh.materialIndex);

            if (ImGui::DragInt("Material Index", &materialIndex, 1.0f, 0, 1024))
                subMesh.materialIndex = static_cast<uint32_t>(materialIndex);

            ImGui::TreePop();
        }

        ImGui::PopID();
    }
}

bool WindowAsset::DrawStringProperty(const char* _label, std::string& _value, size_t _bufferSize)
{
    std::vector<char> buffer(_bufferSize, '\0');

    std::snprintf(buffer.data(), buffer.size(), "%s", _value.c_str());

    if (ImGui::InputText(_label, buffer.data(), buffer.size()))
    {
        _value = buffer.data();
        return true;
    }

    return false;
}

bool WindowAsset::PassSearch(const std::string& _typeName, const std::string& _path, uint32_t _id) const
{
    if (m_searchBuffer[0] == '\0')
        return true;

    const std::string search = ToLower(m_searchBuffer);
    const std::string typeName = ToLower(_typeName);
    const std::string path = ToLower(_path);
    const std::string id = std::to_string(_id);

    return typeName.find(search) != std::string::npos ||
        path.find(search) != std::string::npos ||
        id.find(search) != std::string::npos;
}

std::string WindowAsset::GetTypeDisplayName(std::type_index _type) const
{
    auto it = m_typeDisplayNames.find(_type);

    if (it != m_typeDisplayNames.end())
        return it->second;

    return GetReadableTypeName(_type);
}

std::string WindowAsset::GetReadableTypeName(std::type_index _type) const
{
    return StripCppTypeName(_type.name());
}