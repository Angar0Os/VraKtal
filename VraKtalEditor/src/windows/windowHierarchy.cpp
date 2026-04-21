#include "./windows/windowHierarchy.h"
#include "../../include/imGuiWindows.h"
#include "../../include/windows/others/imGuizmoHelper.h"

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


#include <scene/scene.h>
#include <scene/timeline/entities/mesh.h>
#include <graphics/renderer.h>
#include <core/gpu/buffer.h>

WindowHierarchy::WindowHierarchy(Scene& _scene, graphics::Renderer& _renderer, ImGuiWindows& _imGuiWindows) : m_scene(_scene), m_renderer(_renderer), m_imGuiWindows(_imGuiWindows)
{
}

WindowHierarchy::~WindowHierarchy()
{
}

void WindowHierarchy::Draw()
{
    ComponentStorage<timeline::MeshInstance>& meshStorage = m_scene.GetComponentStorage<timeline::MeshInstance>();
    m_imGuiWindows.BeginWindow("Hierarchy", true);

	timeline::MeshInstance* selectedMesh = nullptr;
	for (EntityID ID : m_scene.GetAliveEntities())
	{
        if (meshStorage.Has(ID))
        {
        	selectedMesh = &meshStorage.Get(ID);
            std::string label = "Entity " + std::to_string(ID);
            bool isSelected = (m_imGuiWindows.selectedItem == ID);

            if (isSelected)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.5f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.3f, 0.9f, 1.0f));
            }

            if (ImGui::Button(label.c_str()))
            {
                if (m_imGuiWindows.selectedItem == ID)
                    m_imGuiWindows.selectedItem = -1; // ou une valeur invalide
                else
                    m_imGuiWindows.selectedItem = ID;
            }

            if (isSelected)
            {
                ImGui::PopStyleColor(3);
            }
        }
    }

    m_imGuiWindows.EndWindow("Hierarchy");
}


bool WindowHierarchy::DrawVec3Control(const char* label, glm::vec3& value, float resetValue = 0.0f, float columnWidth = 100.0f)
{
        bool changed = false;

        ImGui::PushID(label);
        ImGui::Columns(2, nullptr, false);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::TextUnformatted(label);
        ImGui::NextColumn();

        float fullWidth = ImGui::CalcItemWidth();
        ImGuiStyle& style = ImGui::GetStyle();
        float totalGaps = style.ItemSpacing.x * 2.0f;
        float itemWidth = (fullWidth - totalGaps) / 3.0f;
        if (itemWidth < 1.0f) itemWidth = fullWidth;

        ImGui::PushItemWidth(itemWidth);
        ImGui::PushItemWidth(itemWidth);
        ImGui::PushItemWidth(itemWidth);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

        float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
        ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

        if (ImGui::Button("X", buttonSize))
        {
            value.x = resetValue;
            changed = true;
        }
        ImGui::SameLine();
        changed |= ImGui::DragFloat("##X", &value.x, 0.1f);
        ImGui::PopItemWidth();
        ImGui::SameLine();

        if (ImGui::Button("Y", buttonSize))
        {
            value.y = resetValue;
            changed = true;
        }
        ImGui::SameLine();
        changed |= ImGui::DragFloat("##Y", &value.y, 0.1f);
        ImGui::PopItemWidth();
        ImGui::SameLine();

        if (ImGui::Button("Z", buttonSize))
        {
            value.z = resetValue;
            changed = true;
        }
        ImGui::SameLine();
        changed |= ImGui::DragFloat("##Z", &value.z, 0.1f);
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();
        ImGui::Columns(1);
        ImGui::PopID();

        return changed;
}

bool WindowHierarchy::DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotationDeg, glm::vec3& scale)
{
    using namespace glm;
    
    vec3 skew;
    vec4 perspective;
    quat orientation;
    
    if (!decompose(transform, scale, orientation, translation, skew, perspective))
        return false;
    
    vec3 rotationRad = eulerAngles(orientation);
    rotationDeg = degrees(rotationRad);
    return true;
}

inline glm::mat4 ComposeTransform(const glm::vec3& translation, const glm::vec3& rotationDeg, const glm::vec3& scale)
{
    glm::vec3 rotationRad = glm::radians(rotationDeg);
    
    glm::mat4 rotation =
        glm::yawPitchRoll(rotationRad.y, rotationRad.x, rotationRad.z);
    
    glm::mat4 result = glm::translate(glm::mat4(1.0f), translation)
                     * rotation
                     * glm::scale(glm::mat4(1.0f), scale);
    
    return result;
}

inline void DrawMatrix4ReadOnly(const glm::mat4& m)
{
    const float* data = glm::value_ptr(m);
    for (int row = 0; row < 4; ++row)
    {
        ImGui::Text(
            "[%.3f %.3f %.3f %.3f]",
            data[row],
            data[4 + row],
            data[8 + row],
            data[12 + row]
        );
    }
    }

bool WindowHierarchy::DrawTransformEditor(const char* label, glm::mat4& transform)
{
      bool changed = false;

    glm::vec3 translation{ 0.0f };
    glm::vec3 rotationDeg{ 0.0f };
    glm::vec3 scale{ 1.0f };
    
    if (DecomposeTransform(transform, translation, rotationDeg, scale))
    {
        changed |= DrawVec3Control("Position", translation, 0.0f);
        changed |= DrawVec3Control("Rotation", rotationDeg, 0.0f);
        changed |= DrawVec3Control("Scale", scale, 1.0f);
    
        if (changed)
            transform = ComposeTransform(translation, rotationDeg, scale);
    }
    else
    {
        ImGui::TextDisabled("Impossible de decomposer la matrice.");
    }
    
    ImGui::Separator();
    ImGui::TextUnformatted("Matrix");
    DrawMatrix4ReadOnly(transform);

    return changed;
}

void WindowHierarchy::DrawMeshInstanceProperties(timeline::MeshInstance& currentMesh)
{
    if (currentMesh.mesh)
    {
        ImGui::Text("mesh: %p", (void*)currentMesh.mesh);
    }
    else
    {
        ImGui::TextDisabled("mesh: null");
    }
    ImGui::Separator();
    DrawTransformEditor("Temp Transform", currentMesh.temp_transform);
}

void WindowHierarchy::DrawGuizmo(timeline::MeshInstance& object, const glm::mat4& cameraView, const glm::mat4& cameraProjection)
{


    
}