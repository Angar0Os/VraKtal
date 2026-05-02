#include "../../../include/windows/others/imGuizmoHelper.h"

#include "../../../include/imGuiWindows.h"

#include <scene/timeline/components/mesh.h>
#include <scene/scene.h>

#include <core/input/input.h>
#include <core/input/keys.h>
#include <core/gpu/imguiContext.h>

#include <scene/timeline/components/light.h>

#include <imGuizmo/ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <core/gpu/buffer.h>
#include <glm/fwd.hpp>

#include <variant>

ImGuizmoHelper::ImGuizmoHelper(ImGuiWindows* _imGuiWindows, core::Input* _input) : m_imGuiWindows(*_imGuiWindows)
{
    _input->AddAction("Translate");
    _input->BindActionKey({ input::Key::T }, "Translate");
    _input->BindActionCallback<GuizmoSettings , &GuizmoSettings::SetOperationToTranslate>("Translate", &m_settings, input::KeyState::Press);

    _input->AddAction("Rotate");
    _input->BindActionKey({ input::Key::R }, "Rotate");
    _input->BindActionCallback<GuizmoSettings, &GuizmoSettings::SetOperationToRotate>("Rotate", &m_settings, input::KeyState::Press);

    _input->AddAction("Scale");
    _input->BindActionKey({ input::Key::S }, "Scale");
    _input->BindActionCallback<GuizmoSettings, &GuizmoSettings::SetOperationToScale>("Scale", &m_settings, input::KeyState::Press);

}

ImGuizmoHelper::~ImGuizmoHelper()
{
}

void ImGuizmoHelper::DrawGuizmo()
{
    EntityID selectedEntity = m_imGuiWindows.IsSelectedItemType<EntityID>() ? m_imGuiWindows.GetSelectedItem<EntityID>() : INVALID_ENTITY;

    if (selectedEntity == INVALID_ENTITY)
    {
        return;
    }

    auto viewport = m_imGuiWindows.GetContext()->GetViewportState();
    if (! viewport->width <= 0 && !viewport->height <= 0)
    {
        ImGuizmo::BeginFrame();
        DrawGuizmoToolbar();
        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(viewport->posX, viewport->posY, viewport->width, viewport->height);

        glm::mat4 proj = m_imGuiWindows.GetContext()->GetViewportProjection();
        proj[1][1] *= -1.0f;

        if (m_imGuiWindows.GetScene()->GetComponentStorage<timeline::MeshInstance>().Has(selectedEntity))
        {
            ImGuizmo::Manipulate(glm::value_ptr(m_imGuiWindows.GetView()), glm::value_ptr(proj)
                , m_settings.currentOperation, m_settings.currentMode,
                glm::value_ptr(m_imGuiWindows.GetScene()->GetComponentStorage<timeline::MeshInstance>().Get(selectedEntity).temp_properties.transform), nullptr, nullptr);
        }

        if (m_imGuiWindows.GetScene()->GetComponentStorage<timeline::Light>().Has(selectedEntity))
        {
            timeline::Light& light = m_imGuiWindows.GetScene()->GetComponentStorage<timeline::Light>().Get(selectedEntity);

            glm::mat4 lightTransform = glm::translate(glm::mat4(1.0f), light.temp_property.position);

            ImGuizmo::Manipulate(
                glm::value_ptr(m_imGuiWindows.GetView()),
                glm::value_ptr(proj),
                m_settings.currentOperation,
                m_settings.currentMode,
                glm::value_ptr(lightTransform),
                nullptr,
                nullptr
            );

            if (ImGuizmo::IsUsing())
            {
                light.temp_property.position = glm::vec3(lightTransform[3]);
            }
        }
    }
    m_matrices.clear();
}

void ImGuizmoHelper::AddMatriceToEdit(glm::mat4* _matrice)
{
    m_matrices.push_back(_matrice);
}

void ImGuizmoHelper::DrawGuizmoToolbar()
{
    ImGui::SetNextWindowPos(ImVec2(
        m_imGuiWindows.GetContext()->GetViewportState()->posX + 10.0f,
        m_imGuiWindows.GetContext()->GetViewportState()->posY + 10.0f
    ));

    ImGui::SetNextWindowBgAlpha(0.35f);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_AlwaysAutoResize;

    if (ImGui::Begin("##GuizmoToolbar", nullptr, flags))
    {
        auto ToggleButton = [](const char* label, bool active)
            {
                if (active)
                    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));

                bool pressed = ImGui::Button(label);

                if (active)
                    ImGui::PopStyleColor();

                return pressed;
            };

        if (ToggleButton("T", m_settings.currentOperation == ImGuizmo::TRANSLATE))
            m_settings.currentOperation = ImGuizmo::TRANSLATE;

        ImGui::SameLine();

        if (ToggleButton("R", m_settings.currentOperation == ImGuizmo::ROTATE))
            m_settings.currentOperation = ImGuizmo::ROTATE;

        ImGui::SameLine();

        if (ToggleButton("S", m_settings.currentOperation == ImGuizmo::SCALE))
            m_settings.currentOperation = ImGuizmo::SCALE;
    }

    ImGui::End();
}