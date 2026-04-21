#include "../../../include/windows/others/imGuizmoHelper.h"

#include "../../../include/imGuiWindows.h"

#include <core/input/input.h>
#include <core/input/keys.h>
#include <core/gpu/imguiContext.h>
#include <scene/scene.h>

#include <imGuizmo/ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <core/gpu/buffer.h>
#include <glm/fwd.hpp>

#include <scene/timeline/entities/mesh.h>


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
    auto viewport = m_imGuiWindows.GetContext()->GetViewportState();
    if (! viewport->width <= 0 && !viewport->height <= 0)
    {
        ImGuizmo::BeginFrame();
        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(viewport->posX, viewport->posY, viewport->width, viewport->height);

        glm::mat4 proj = m_imGuiWindows.GetContext()->GetViewportProjection();
        proj[1][1] *= -1.0f;

        if (m_imGuiWindows.GetScene()->GetComponentStorage<timeline::MeshInstance>().Has(m_imGuiWindows.selectedItem))
        {
            ImGuizmo::Manipulate(glm::value_ptr(m_imGuiWindows.GetView()), glm::value_ptr(proj)
                , m_settings.currentOperation, m_settings.currentMode,
                glm::value_ptr(m_imGuiWindows.GetScene()->GetComponentStorage<timeline::MeshInstance>().Get(m_imGuiWindows.selectedItem).temp_transform), nullptr, nullptr);
        }
    }
    m_matrices.clear();
}

void ImGuizmoHelper::AddMatriceToEdit(glm::mat4* _matrice)
{
    m_matrices.push_back(_matrice);
}
