#define NOMINMAX
#include "imGuiWindows.h"
#include <core/gpu/imguiContext.h>
#include "imgui/imgui.h"
#include "contentDrawer.h"
#include "command/fileCommands.h"
#include "command/sceneCommands.h"
#include <core/gpu/buffer.h>
#include <graphics/resources/object/camera.h>
#include <graphics/resources/object/light.h>
#include <graphics/resources/object/mesh.h>
#include <algorithm>
#include <array>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.inl>
#include <glm/gtx/matrix_decompose.inl>

#include <imGuizmo/ImGuizmo.h>
#include <imgui/imgui.h>

#include "portable-file-dialogs/portable-file-dialogs.h"

#include <iostream>
#include <MDI/IconsMaterialDesignIcons.h>

ImGuiWindows::ImGuiWindows(core::gpu::ImguiContext* _imGuiContext,
    graphics::Renderer* _renderer,
    core::Window* window,
    demo::Scene* scene,
    loaders::MeshLoader* meshLoader)
    : m_imGuiContext(_imGuiContext)
    , m_scene(scene)
    , m_meshLoader(meshLoader)
{
    m_renderer = _renderer;
    m_window = window;

    command::ClearBackupDirectory();
    m_commandHistory = std::make_unique<command::CommandHistory>(100);
    m_contentDrawer.SetCommandHistory(m_commandHistory.get());
}

ImGuiWindows::~ImGuiWindows()
{
    command::ClearBackupDirectory();
}

void ImGuiWindows::PrepareImGuiWindows()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    MainWindow();
    Viewport();
    TestWindow();
    ContentDrawerWindow();
    HierarchyWindow();
    InspectorWindow();

    m_newProjectModal.GetNewProjectModalWindow();

    if (m_newProjectModal.HasNewProjectCreated())
    {
        std::filesystem::path lastProjectPath = m_newProjectModal.GetLastCreatedProjectPath();
        if (!lastProjectPath.empty())
            m_contentDrawer.SetCurrentPath(lastProjectPath);
        m_newProjectModal.ResetProjectCreatedFlag();
    }

    if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z))
        if (m_commandHistory->CanUndo()) m_commandHistory->Undo();

    if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y))
        if (m_commandHistory->CanRedo()) m_commandHistory->Redo();
}

core::gpu::ImguiContext* ImGuiWindows::GetContext()
{
    return m_imGuiContext;
}

void ImGuiWindows::ContentDrawerWindow()
{
    m_contentDrawer.GetContentDrawerWindow();
}

void ImGuiWindows::HierarchyWindow()
{
    ImGui::Begin(ICON_MDI_VIEW_LIST " Hierarchy");

    if (ImGui::Button(ICON_MDI_PLUS " Add"))
        ImGui::OpenPopup("add_object_popup");

    if (ImGui::BeginPopup("add_object_popup"))
    {
        if (m_scene)
        {
            if (ImGui::MenuItem(ICON_MDI_CAMERA " Camera"))
            {
                auto* cam = new graphics::resources::object::Camera("NewCamera");
                demo::SceneResource res;
                res.objectName = "NewCamera";
                res.object = cam;
                res.isInTimeline = false;

                m_commandHistory->ExecuteCommand(
                    std::make_unique<command::AddSceneObjectCommand>(m_scene, std::move(res))
                );
            }

            if (ImGui::BeginMenu(ICON_MDI_LIGHTBULB_OUTLINE " Light"))
            {
                if (ImGui::MenuItem("Point Light"))
                {
                    auto light = graphics::resources::Light::CreatePointLight(
                        glm::vec3(0.0f), glm::vec3(1.0f), 10.0f, "NewPointLight");

                    demo::SceneResource res;
                    res.objectName = "NewPointLight";
                    res.object = new graphics::resources::Light(light);
                    res.isInTimeline = false;

                    m_commandHistory->ExecuteCommand(
                        std::make_unique<command::AddSceneObjectCommand>(m_scene, std::move(res))
                    );
                }
                ImGui::EndMenu();
            }

            if (ImGui::MenuItem(ICON_MDI_CUBE_OUTLINE " Empty Object"))
            {
                demo::SceneResource res;
                res.objectName = "NewObject";
                res.object = static_cast<graphics::resources::Mesh*>(nullptr);
                res.isInTimeline = false;

                m_commandHistory->ExecuteCommand(
                    std::make_unique<command::AddSceneObjectCommand>(m_scene, std::move(res))
                );
            }
        }
        ImGui::EndPopup();
    }

    ImGui::Separator();

    if (m_scene)
    {
        for (size_t i = 0; i < m_scene->sceneObjects.size(); ++i)
        {
            auto& res = m_scene->sceneObjects[i];

            const char* icon = ICON_MDI_CUBE_OUTLINE;
            std::visit([&](auto* obj)
                {
                    using T = std::decay_t<decltype(*obj)>;
                    if constexpr (std::is_same_v<T, graphics::resources::object::Camera>)
                        icon = ICON_MDI_CAMERA;
                    else if constexpr (std::is_same_v<T, graphics::resources::Light>)
                        icon = ICON_MDI_LIGHTBULB_OUTLINE;
                }, res.object);

            std::string label = std::string(icon) + " " + res.objectName + "##" + std::to_string(i);

            bool selected = (m_selectedObjectIndex.has_value() && *m_selectedObjectIndex == i);
            if (ImGui::Selectable(label.c_str(), selected))
                m_selectedObjectIndex = i;

            if (ImGui::BeginPopupContextItem(("ctx##" + std::to_string(i)).c_str()))
            {
                if (ImGui::MenuItem(ICON_MDI_DELETE " Delete"))
                {
                    m_commandHistory->ExecuteCommand(
                        std::make_unique<command::RemoveSceneObjectCommand>(m_scene, i)
                    );
                    if (m_selectedObjectIndex.has_value() && *m_selectedObjectIndex == i)
                        m_selectedObjectIndex.reset();
                }
                ImGui::EndPopup();
            }
        }
    }
    else
    {
        ImGui::TextDisabled("No scene loaded.");
    }

    ImGui::End();
}

void ImGuiWindows::InspectorWindow()
{
    ImGui::Begin(ICON_MDI_TUNE " Inspector");

    if (!m_scene || !m_selectedObjectIndex.has_value())
    {
        ImGui::TextDisabled("Select an object in the Hierarchy.");
        ImGui::End();
        return;
    }

    size_t idx = *m_selectedObjectIndex;
    if (idx >= m_scene->sceneObjects.size())
    {
        m_selectedObjectIndex.reset();
        ImGui::End();
        return;
    }

    auto& res = m_scene->sceneObjects[idx];

    if (ImGui::CollapsingHeader(ICON_MDI_TAG_OUTLINE " Identity", ImGuiTreeNodeFlags_DefaultOpen))
    {
        std::array<char, 256> nameBuf{};
        size_t copyLength = std::min(res.objectName.size(), nameBuf.size() - 1);
        res.objectName.copy(nameBuf.data(), copyLength);

        ImGui::SetNextItemWidth(-1.0f);
        if (ImGui::InputText("##name", nameBuf.data(), nameBuf.size(),
            ImGuiInputTextFlags_EnterReturnsTrue))
        {
            std::string newName(nameBuf.data());
            if (!newName.empty() && newName != res.objectName)
            {
                m_commandHistory->ExecuteCommand(
                    std::make_unique<command::RenameSceneObjectCommand>(m_scene, idx, newName)
                );
            }
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(Enter to confirm)");

        bool inTimeline = res.isInTimeline;
        if (ImGui::Checkbox("In Timeline", &inTimeline))
        {
            m_commandHistory->ExecuteCommand(
                std::make_unique<command::SetTimelineCommand>(m_scene, idx, inTimeline)
            );
        }
    }

    InspectorTransform(idx);

    std::visit([&](auto* obj)
        {
            using T = std::decay_t<decltype(*obj)>;
            if constexpr (std::is_same_v<T, graphics::resources::object::Camera>)
                InspectorCameraProperties(idx);
            else if constexpr (std::is_same_v<T, graphics::resources::Light>)
                InspectorLightProperties(idx);
        }, res.object);

    ImGui::End();
}

void ImGuiWindows::InspectorTransform(size_t index)
{
    if (!ImGui::CollapsingHeader(ICON_MDI_AXIS_ARROW " Transform", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    auto& transform = m_scene->sceneObjects[index].objectTransform;

    glm::mat4 mat = transform.GetMatrix();
    glm::vec3 scale, skew, translation;
    glm::quat rotation;
    glm::vec4 perspective;
    glm::decompose(mat, scale, rotation, translation, skew, perspective);

    glm::vec3 eulerDeg = glm::degrees(glm::eulerAngles(rotation));

    bool changed = false;

    ImGui::Text("Position");
    ImGui::SetNextItemWidth(-1.0f);
    changed |= ImGui::DragFloat3("##pos", glm::value_ptr(translation), 0.1f);

    ImGui::Text("Rotation (deg)");
    ImGui::SetNextItemWidth(-1.0f);
    changed |= ImGui::DragFloat3("##rot", glm::value_ptr(eulerDeg), 1.0f);

    ImGui::Text("Scale");
    ImGui::SetNextItemWidth(-1.0f);
    changed |= ImGui::DragFloat3("##scl", glm::value_ptr(scale), 0.01f, 0.001f, 1000.0f);

    if (changed)
    {
        graphics::resources::property::Transform newTransform;
        newTransform.SetPosition(translation);
        newTransform.SetRotation(glm::quat(glm::radians(eulerDeg)));
        newTransform.SetScale(scale);

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            m_commandHistory->ExecuteCommand(
                std::make_unique<command::ModifyTransformCommand>(m_scene, index, newTransform)
            );
        }
        else
        {
            m_scene->sceneObjects[index].objectTransform = newTransform;
        }
    }
}

void ImGuiWindows::InspectorCameraProperties(size_t index)
{
    if (!ImGui::CollapsingHeader(ICON_MDI_CAMERA " Camera", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    auto* cam = std::get<graphics::resources::object::Camera*>(
        m_scene->sceneObjects[index].object);

    if (!cam) return;

    ImGui::DragFloat("FOV", &cam->fov, 0.5f, 5.0f, 170.0f);
    ImGui::DragFloat("Aspect Ratio", &cam->aspectRatio, 0.001f, 0.1f, 4.0f);
    ImGui::DragFloat("Near Clip", &cam->zNear, 0.001f, 0.001f, 10.0f);
    ImGui::DragFloat("Far Clip", &cam->zFar, 1.0f, 1.0f, 100000.0f);

    bool isActive = (m_scene->GetActiveCamera() == cam);
    if (ImGui::Checkbox("Active Camera", &isActive))
    {
        if (isActive)
            m_scene->SetActiveCamera(index);
    }
}

void ImGuiWindows::InspectorLightProperties(size_t index)
{
    if (!ImGui::CollapsingHeader(ICON_MDI_LIGHTBULB_OUTLINE " Light", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    auto* light = std::get<graphics::resources::Light*>(
        m_scene->sceneObjects[index].object);

    if (!light) return;

    ImGui::ColorEdit3("Color", glm::value_ptr(light->color));
    ImGui::DragFloat("Intensity", &light->intensity, 0.1f, 0.0f, 1000.0f);
    ImGui::DragFloat("Radius", &light->radius, 0.01f, 0.0f, 100.0f);
    ImGui::Checkbox("Enabled", &light->enabled);
}

void ImGuiWindows::Viewport()
{
    ImGui::Begin("Viewport");

    ImVec2 avail = ImGui::GetContentRegionAvail();
    uint32_t width = std::max(1u, static_cast<uint32_t>(avail.x));
    uint32_t height = std::max(1u, static_cast<uint32_t>(avail.y));

    m_imGuiContext->DrawViewportComponent(width, height);

    ImGui::End();
}

void ImGuiWindows::TestWindow() {}

void ImGuiWindows::MainWindow()
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("DockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

    SetMenuBar();
    ImGui::End();
}

void ImGuiWindows::SetMenuBar()
{
    if (!ImGui::BeginMenuBar()) return;

    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("New Project"))
            m_newProjectModal.ToggleNewProjectModal();

        if (ImGui::MenuItem("Open Project"))
            LoadProject();

        if (ImGui::MenuItem("Save Project")) {}

        if (ImGui::MenuItem("Quit"))
            if (m_window) m_window->Close();

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit"))
    {
        bool canUndo = m_commandHistory->CanUndo();
        std::string undoText = "Undo";
        if (canUndo)
            undoText += " : " + m_commandHistory->GetCommandDescription(m_commandHistory->GetCurrentIndex());

        if (ImGui::MenuItem((ICON_MDI_UNDO " " + undoText).c_str(), "Ctrl+Z", false, canUndo))
            m_commandHistory->Undo();

        bool canRedo = m_commandHistory->CanRedo();
        std::string redoText = "Redo";
        if (canRedo)
            redoText += " : " + m_commandHistory->GetCommandDescription(m_commandHistory->GetCurrentIndex() + 1);

        if (ImGui::MenuItem((ICON_MDI_REDO " " + redoText).c_str(), "Ctrl+Y", false, canRedo))
            m_commandHistory->Redo();

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Tools"))
    {
        if (ImGui::MenuItem("Timeline")) {}
        if (ImGui::MenuItem("CameraSplineEditor")) {}
        if (ImGui::MenuItem("Tracy")) {}
        ImGui::EndMenu();
    }

    if (ImGui::MenuItem("Build"))
        ImGui::OpenPopup("build_popup");

    if (ImGui::BeginPopup("build_popup"))
    {
        if (ImGui::Button("Build")) {}; ImGui::SameLine();
        if (ImGui::Button("Build & Run")) {}
        ImGui::EndPopup();
    }

    ImGui::EndMenuBar();
}

void ImGuiWindows::LoadProject()
{
    auto selection = pfd::open_file(
        "Choose a project", "",
        { "YAML", "*.yaml" }
    );

    auto files = selection.result();
    if (files.empty()) return;

    std::filesystem::path projectPath(files[0]);
    if (!projectPath.parent_path().empty())
        m_contentDrawer.SetCurrentPath(projectPath.parent_path());
}

void ImGuiWindows::EditTransformByIndice(const float*, const float*, int) {}