#pragma once
#include <scene/timeline/entities/mesh.h>
#include <imgui/imgui.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <limits>


struct Inspect
{
    template<typename T>
    static void Draw(T& object) {
        ImGui::Text("No inspector available for this type.");
    };
};

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

        ImGui::SeparatorText("Skew");
        DrawPropertyWithReset("R", "##Skew", skew, glm::vec3(0.0f), 0.01f);
    }

    if (changed)
    {
        glm::quat newRotation = glm::quat(glm::radians(rotationEuler));

        glm::mat4 translationMat = glm::translate(glm::mat4(1.0f), translation);
        glm::mat4 rotationMat = glm::mat4_cast(newRotation);
        glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);

        glm::mat4 skewMat(1.0f);
        skewMat[1][0] = skew.x; // XY
        skewMat[2][0] = skew.y; // XZ
        skewMat[2][1] = skew.z; // YZ

        transform = translationMat * rotationMat * skewMat * scaleMat;
    }
}