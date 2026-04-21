#include "./windows/windowHierarchy.h"
#include <scene/scene.h>

#include <imGuizmo/ImGuizmo.h>
#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <scene/timeline/entities/mesh.h>
#include <scene/scene.h>

#include <core/gpu/imguiContext.h>
#include <graphics/renderer.h>
#include <core/gpu/buffer.h>

WindowHierarchy::WindowHierarchy(Scene& _scene, graphics::Renderer& _renderer, ImguiContext& _imguiContext) : m_scene(_scene), m_renderer(_renderer), m_imguiContext(_imguiContext)
{
}

WindowHierarchy::~WindowHierarchy()
{
}

void WindowHierarchy::Draw()
{
	ComponentStorage<timeline::MeshInstance>& meshStorage = m_scene.GetComponentStorage<timeline::MeshInstance>();
	
	timeline::MeshInstance* selectedMesh = nullptr;
	for (EntityID ID : m_scene.GetAliveEntities())
	{
        if (meshStorage.Has(ID))
        {
        	selectedMesh = &meshStorage.Get(ID);
            std::string label = "Entity " + std::to_string(ID);
            if (ImGui::TreeNode(label.c_str()))
            {
                DrawMeshInstanceProperties(*selectedMesh);
                DrawGuizmo(*selectedMesh, m_renderer.GetViewMatrix(), m_renderer.GetProjectionMatrix());
                ImGui::TreePop();

            }
        }
    }
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
    static ImGuizmo::OPERATION currentOperation = ImGuizmo::ROTATE;
    static ImGuizmo::MODE currentMode = ImGuizmo::WORLD;
    static bool useSnap = false;

    if (ImGui::IsKeyPressed(ImGuiKey_T))
        currentOperation = ImGuizmo::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_E))
        currentOperation = ImGuizmo::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R))
        currentOperation = ImGuizmo::SCALE;
    if (ImGui::IsKeyPressed(ImGuiKey_S))
        useSnap = !useSnap;

    if (ImGui::RadioButton("Translate", currentOperation == ImGuizmo::TRANSLATE))
        currentOperation = ImGuizmo::TRANSLATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Rotate", currentOperation == ImGuizmo::ROTATE))
        currentOperation = ImGuizmo::ROTATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Scale", currentOperation == ImGuizmo::SCALE))
        currentOperation = ImGuizmo::SCALE;

    float matrixTranslation[3];
    float matrixRotation[3];
    float matrixScale[3];

    ImGuizmo::DecomposeMatrixToComponents(
        glm::value_ptr(object.temp_transform),
        matrixTranslation,
        matrixRotation,
        matrixScale
    );

    if (ImGui::InputFloat3("Tr", matrixTranslation))
    {
        ImGuizmo::RecomposeMatrixFromComponents(
            matrixTranslation,
            matrixRotation,
            matrixScale,
            glm::value_ptr(object.temp_transform)
        );
    }

    if (ImGui::InputFloat3("Rt", matrixRotation))
    {
        ImGuizmo::RecomposeMatrixFromComponents(
            matrixTranslation,
            matrixRotation,
            matrixScale,
            glm::value_ptr(object.temp_transform)
        );
    }

    if (ImGui::InputFloat3("Sc", matrixScale))
    {
        ImGuizmo::RecomposeMatrixFromComponents(
            matrixTranslation,
            matrixRotation,
            matrixScale,
            glm::value_ptr(object.temp_transform)
        );
    }

    if (currentOperation != ImGuizmo::SCALE)
    {
        if (ImGui::RadioButton("Local", currentMode == ImGuizmo::LOCAL))
            currentMode = ImGuizmo::LOCAL;
        ImGui::SameLine();
        if (ImGui::RadioButton("World", currentMode == ImGuizmo::WORLD))
            currentMode = ImGuizmo::WORLD;
    }

    ImGui::Checkbox("Snap", &useSnap);

    float snapValues[3] = { 1.0f, 1.0f, 1.0f };

    if (currentOperation == ImGuizmo::ROTATE)
    {
        snapValues[0] = 15.0f;
        snapValues[1] = 15.0f;
        snapValues[2] = 15.0f;
    }
    else if (currentOperation == ImGuizmo::SCALE)
    {
        snapValues[0] = 0.1f;
        snapValues[1] = 0.1f;
        snapValues[2] = 0.1f;
    }

    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0.0f, 0.0f, io.DisplaySize.x, io.DisplaySize.y);

    ImGuizmo::Manipulate(
        glm::value_ptr(cameraView),
        glm::value_ptr(cameraProjection),
        currentOperation,
        currentMode,
        glm::value_ptr(object.temp_transform),
        nullptr,
        useSnap ? snapValues : nullptr
    );
}