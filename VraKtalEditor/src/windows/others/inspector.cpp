#include "../../../include/windows/others/inspector.h"
#include "../../../include/windows/others/dragNdrop.h"
#include "../../../include/imGuiWindows.h"

#include <scene/timeline/components/light.h>
#include <scene/timeline/components/mesh.h>

#include <filesystem>

#include <graphics/resources/object/mesh.h>
#include <graphics/resources/material.h>

#include <core/manager/assetManager.h>

#include <memory>
#include <utility>

#pragma region glm
Inspect::Inspect(ImGuiWindows* _windowManager, RessourceManager* _ressourceManager, AssetManager& _astManager)
    : m_ressourceManager(_ressourceManager), m_windowManager(_windowManager) , m_assetManager(_astManager){};

template<>
void Inspect::Draw(glm::mat4& transform)
{
    glm::vec3 translation;
    glm::quat rotation;
    glm::vec3 scale;
    glm::vec3 skew;
    glm::vec4 perspective;

    glm::decompose(transform, scale, rotation, translation, skew, perspective);
    rotation = glm::normalize(rotation);

    glm::vec3 rotationEuler = glm::degrees(glm::eulerAngles(rotation));

    bool changed = false;

    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto DrawPropertyWithReset = [&](const char* buttonLabel,
            const char* dragLabel,
            glm::vec3& value,
            const glm::vec3& resetValue,
            float speed)
            {
                ImGui::PushID(dragLabel);

                if (ImGui::Button(buttonLabel, ImVec2(22.0f, 22.0f)))
                {
                    value = resetValue;
                    changed = true;
                }

                ImGui::SameLine();
                ImGui::SetNextItemWidth(-1.0f);
                changed |= ImGui::DragFloat3(dragLabel, glm::value_ptr(value), speed);

                ImGui::PopID();
            };

        ImGui::SeparatorText("Translation");
        DrawPropertyWithReset("R", "##Translation", translation, glm::vec3(0.0f), 0.01f);

        ImGui::SeparatorText("Rotation");
        DrawPropertyWithReset("R", "##Rotation", rotationEuler, glm::vec3(0.0f), 0.1f);

        ImGui::SeparatorText("Scale");
        DrawPropertyWithReset("R", "##Scale", scale, glm::vec3(1.0f), 0.01f);
    }
    if (changed)
    {
        glm::quat newRotation = glm::quat(glm::radians(rotationEuler));
        glm::mat4 translationMat = glm::translate(glm::mat4(1.0f), translation);
        glm::mat4 rotationMat = glm::mat4_cast(newRotation);
        glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);
        transform = translationMat * rotationMat * scaleMat;
    }
}
#pragma endregion

using MaterialIndex = uint32_t;

template<>
void Inspect::Draw(timeline::MeshInstance& _mesh)
{
    bool bMeshDefined = _mesh.assetID != INVALID_ID;
    std::string path = bMeshDefined ? m_assetManager.GetAsset<graphics::assets::Mesh>(_mesh.assetID).path : "undefined path";

    std::string displayName = "Mesh";

    if (bMeshDefined)
    {
        std::string fileName = std::filesystem::path(path).filename().string();
        displayName += " - " + fileName;
        if (ImGui::CollapsingHeader(displayName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::CollapsingHeader("Materials"))
            {
                graphics::resources::Mesh& ressourceMesh = m_ressourceManager->GetResource<graphics::resources::Mesh>(_mesh.assetID);
                for (int i = 0; i < ressourceMesh.materials.size(); i++)
                {
                    ImGui::PushID(i);

                    MaterialIndex materialIndex = static_cast<MaterialIndex>(i);

                    std::string label =
                        "Material[" + std::to_string(i) + "] " +
                        ressourceMesh.materials[i]->name;

                    ImGui::Selectable(label.c_str(), false);
                    MaterialIndexPayload IndexPayLoad = static_cast<MaterialIndex>(i);
                    m_windowManager->GetDragNDrop()->Drag<MaterialIndexPayload>(IndexPayLoad);

                    MaterialIndex dropped = INVALID_ID;
                    m_windowManager->GetDragNDrop()->DropItem<MaterialIndexPayload, MaterialIndex>(dropped);

                    if (dropped != INVALID_ID && dropped != static_cast<MaterialIndex>(i))
                    {
                        std::swap(
                            ressourceMesh.materials[i],
                            ressourceMesh.materials[dropped]
                        );
                    }

                    ImGui::PopID();
                }
            }
            this->Draw<glm::mat4>(_mesh.temp_properties.transform);
        }
    }
    else
    {
        displayName += " - undefined";
        ImGui::Text(displayName.c_str());
    }
}

template<>
void Inspect::Draw(timeline::Light& _light) 
{
    if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
    {
        timeline::LightProperty& light = _light.temp_property;

        const char* lightTypeItems[] = { "Point", "Directional", "Spot" };
        int currentType = static_cast<int>(light.type);
        if (ImGui::Combo("Type", &currentType, lightTypeItems, IM_ARRAYSIZE(lightTypeItems)))
            light.type = static_cast<timeline::LightType>(currentType);

        ImGui::Checkbox("Enabled", &light.enabled);
        ImGui::ColorEdit3("Color", glm::value_ptr(light.color));
        ImGui::DragFloat("Intensity", &light.intensity, 0.01f, 0.0f, 1000.0f);
        ImGui::DragFloat3("Position", glm::value_ptr(light.position), 0.01f);
        ImGui::DragFloat3("Direction", glm::value_ptr(light.direction), 0.01f);

        if (light.type == timeline::LightType::Spot)
        {
            ImGui::DragFloat("Inner Cone Angle", &light.innerConeAngle, 0.1f, 0.0f, 180.0f);
            ImGui::DragFloat("Outer Cone Angle", &light.outerConeAngle, 0.1f, 0.0f, 180.0f);
        }

        if (light.type == timeline::LightType::Point || light.type == timeline::LightType::Spot)
        {
            ImGui::DragFloat("Constant", &light.constant, 0.001f, 0.0f, 100.0f);
            ImGui::DragFloat("Linear", &light.linear, 0.001f, 0.0f, 100.0f);
            ImGui::DragFloat("Quadratic", &light.quadratic, 0.001f, 0.0f, 100.0f);
            ImGui::DragFloat("Radius", &light.radius, 0.01f, 0.0f, 10000.0f);
            ImGui::DragFloat("Light Radius", &light.lightRadius, 0.01f, 0.0f, 10000.0f);
        }
    }
}

