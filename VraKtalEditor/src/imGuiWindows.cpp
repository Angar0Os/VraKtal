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
#include <cstring>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.inl>
#include <glm/gtx/matrix_decompose.inl>

#include <imGuizmo/ImGuizmo.h>
#include <imgui/imgui.h>

#include "portable-file-dialogs/portable-file-dialogs.h"
#include <utils/yamlSerializer.h>
#include <utils/yamlParser.h>

#include <iostream>
#include <MDI/IconsMaterialDesignIcons.h>

ImGuiWindows::ImGuiWindows(core::gpu::ImguiContext* _imGuiContext,
    graphics::Renderer* _renderer,
    core::Window* window,
    std::vector<demo::Scene>* scenes,
    loaders::MeshLoader* meshLoader)
    : m_imGuiContext(_imGuiContext)
    , m_scenes(scenes)
    , m_meshLoader(meshLoader)
{
    m_renderer = _renderer;
    m_window = window;

    if (m_scenes && !m_scenes->empty())
    {
        m_activeSceneIndex = 0;
    }

    command::ClearBackupDirectory();
    m_commandHistory = std::make_unique<command::CommandHistory>(100);
    m_contentDrawer.SetCommandHistory(m_commandHistory.get());
}

ImGuiWindows::~ImGuiWindows()
{
    command::ClearBackupDirectory();
}

demo::Scene* ImGuiWindows::GetActiveScene() const
{
    if (!m_scenes || m_scenes->empty() || !m_activeSceneIndex.has_value())
        return nullptr;
    if (*m_activeSceneIndex >= m_scenes->size())
        return nullptr;
    return &(*m_scenes)[*m_activeSceneIndex];
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

    ImGui::SameLine();

    if (ImGui::Button(ICON_MDI_PLUS_BOX_OUTLINE " Add Scene"))
    {
        if (m_scenes)
        {
            std::string newName = "Scene_" + std::to_string(m_scenes->size() + 1);
            m_scenes->emplace_back(newName);

            size_t newIdx = m_scenes->size() - 1;
            m_activeSceneIndex = newIdx;
            m_editingSceneIndex = newIdx;
            m_selectedObjectIndex.reset();

            demo::Scene& newScene = (*m_scenes)[newIdx];
            auto* defaultCam = new graphics::resources::object::Camera("Camera");
            defaultCam->fov = 45.0f;
            defaultCam->aspectRatio = 16.0f / 9.0f;
            defaultCam->transform.SetPosition(glm::vec3(0.0f, 3.0f, -5.0f));
            defaultCam->LookAt(glm::vec3(0.0f, 0.0f, 0.0f));

            demo::SceneResource camRes;
            camRes.objectName = "Camera";
            camRes.object = defaultCam;
            camRes.isInTimeline = false;
            newScene.Add(std::move(camRes));
        }
    }

    if (ImGui::BeginPopup("add_object_popup"))
    {
        demo::Scene* activeScene = GetActiveScene();
        if (activeScene)
        {
            if (ImGui::MenuItem(ICON_MDI_CAMERA " Camera"))
            {
                auto* cam = new graphics::resources::object::Camera("NewCamera");
                demo::SceneResource res;
                res.objectName = "NewCamera";
                res.object = cam;
                res.isInTimeline = false;

                m_commandHistory->ExecuteCommand(
                    std::make_unique<command::AddSceneObjectCommand>(activeScene, std::move(res))
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
                        std::make_unique<command::AddSceneObjectCommand>(activeScene, std::move(res))
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
                    std::make_unique<command::AddSceneObjectCommand>(activeScene, std::move(res))
                );
            }
        }
        else
        {
            ImGui::TextDisabled("No active scene.");
        }
        ImGui::EndPopup();
    }

    ImGui::Separator();

    if (!m_scenes || m_scenes->empty())
    {
        ImGui::TextDisabled("No scene. Click \"Add Scene\".");
        ImGui::End();
        return;
    }

    for (size_t i = 0; i < m_scenes->size(); ++i)
    {
        demo::Scene& scene = (*m_scenes)[i];
        bool         isActive = m_activeSceneIndex.has_value() && *m_activeSceneIndex == i;

        if (m_editingSceneIndex.has_value() && *m_editingSceneIndex == i)
        {
            char buf[128]{};
            size_t copyLength = std::min(scene.name.size(), sizeof(buf) - 1);
            scene.name.copy(buf, copyLength);

            ImGui::SetNextItemWidth(160.0f);
            ImGui::SetKeyboardFocusHere();

            if (ImGui::InputText(("##rename_scene_" + std::to_string(i)).c_str(),
                buf, sizeof(buf),
                ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
            {
                if (std::strlen(buf) > 0)
                    scene.name = buf;
                m_editingSceneIndex.reset();
            }

            if (!ImGui::IsItemActive() && ImGui::IsMouseClicked(0))
            {
                if (std::strlen(buf) > 0)
                    scene.name = buf;
                m_editingSceneIndex.reset();
            }

            continue;
        }

        ImGuiTreeNodeFlags nodeFlags =
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_SpanAvailWidth;

        if (isActive)
        {
            nodeFlags |= ImGuiTreeNodeFlags_Selected;
        }

        if (!isActive)
        {
            nodeFlags |= ImGuiTreeNodeFlags_Leaf;
        }

        if (isActive)
        {
            ImGui::SetNextItemOpen(true, ImGuiCond_Always);
        }

        bool opened = ImGui::TreeNodeEx(
            (void*)(intptr_t)i,
            nodeFlags,
            "%s  %s",
            isActive ? ICON_MDI_MOVIE_OPEN : ICON_MDI_MOVIE,
            scene.name.c_str()
        );

        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
        {
            m_activeSceneIndex = i;
            m_selectedObjectIndex.reset();
        }

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            m_editingSceneIndex = i;
        }

        if (ImGui::BeginPopupContextItem(("ctx_scene_" + std::to_string(i)).c_str()))
        {
            if (ImGui::MenuItem(ICON_MDI_PENCIL " Rename"))
                m_editingSceneIndex = i;

            ImGui::Separator();

            bool canDelete = m_scenes->size() > 1;
            if (!canDelete)
            {
                ImGui::BeginDisabled();
            }

            if (ImGui::MenuItem(ICON_MDI_DELETE " Delete Scene"))
            {
                m_pendingDeleteSceneIndex = i;
                ImGui::OpenPopup("confirm_delete_scene");
            }

            if (!canDelete)
            {
                ImGui::EndDisabled();
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    ImGui::SetTooltip("Cannot delete the last scene.");
            }

            ImGui::EndPopup();
        }

        if (opened)
        {
            if (isActive)
            {
                for (size_t j = 0; j < scene.sceneObjects.size(); ++j)
                {
                    auto& res = scene.sceneObjects[j];

                    const char* icon = ICON_MDI_CUBE_OUTLINE;
                    std::visit([&](auto* obj)
                        {
                            using T = std::decay_t<decltype(*obj)>;
                            if constexpr (std::is_same_v<T, graphics::resources::object::Camera>)
                                icon = ICON_MDI_CAMERA;
                            else if constexpr (std::is_same_v<T, graphics::resources::Light>)
                                icon = ICON_MDI_LIGHTBULB_OUTLINE;
                        }, res.object);

                    ImGuiTreeNodeFlags objFlags =
                        ImGuiTreeNodeFlags_Leaf |
                        ImGuiTreeNodeFlags_NoTreePushOnOpen |
                        ImGuiTreeNodeFlags_SpanAvailWidth;

                    if (m_selectedObjectIndex.has_value() && *m_selectedObjectIndex == j)
                        objFlags |= ImGuiTreeNodeFlags_Selected;

                    std::string objLabel = std::string(icon) + " " + res.objectName
                        + "##obj_" + std::to_string(j);

                    ImGui::TreeNodeEx((void*)(intptr_t)j, objFlags, "%s", objLabel.c_str());

                    if (ImGui::IsItemClicked())
                    {
                        m_selectedObjectIndex = j;
                    }

                    if (ImGui::BeginPopupContextItem(("ctx_obj_" + std::to_string(j)).c_str()))
                    {
                        if (ImGui::MenuItem(ICON_MDI_DELETE " Delete"))
                        {
                            m_commandHistory->ExecuteCommand(
                                std::make_unique<command::RemoveSceneObjectCommand>(&scene, j)
                            );
                            if (m_selectedObjectIndex.has_value() && *m_selectedObjectIndex == j)
                                m_selectedObjectIndex.reset();
                        }
                        ImGui::EndPopup();
                    }
                }
            }
            ImGui::TreePop();
        }
    }

    if (m_pendingDeleteSceneIndex.has_value())
    {
        ImGui::OpenPopup("confirm_delete_scene");
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("confirm_delete_scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (m_pendingDeleteSceneIndex.has_value())
        {
            const std::string& sceneName = (*m_scenes)[*m_pendingDeleteSceneIndex].name;
            ImGui::Text(ICON_MDI_ALERT_OUTLINE "  Delete scene \"%s\" ?", sceneName.c_str());
            ImGui::Text("This action cannot be undone.");
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button(ICON_MDI_DELETE " Delete", ImVec2(120, 0)))
            {
                size_t idx = *m_pendingDeleteSceneIndex;

                m_scenes->erase(m_scenes->begin() + idx);

                if (m_scenes->empty())
                {
                    m_activeSceneIndex.reset();
                }
                else if (m_activeSceneIndex.has_value())
                {
                    if (*m_activeSceneIndex == idx)
                        m_activeSceneIndex = (idx > 0) ? idx - 1 : 0;
                    else if (*m_activeSceneIndex > idx)
                        --(*m_activeSceneIndex);
                }

                m_selectedObjectIndex.reset();
                m_editingSceneIndex.reset();
                m_pendingDeleteSceneIndex.reset();
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                m_pendingDeleteSceneIndex.reset();
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }

    ImGui::End();
}

void ImGuiWindows::InspectorWindow()
{
    ImGui::Begin(ICON_MDI_TUNE " Inspector");

    demo::Scene* activeScene = GetActiveScene();

    if (!activeScene || !m_selectedObjectIndex.has_value())
    {
        ImGui::TextDisabled("Select an object in the Hierarchy.");
        ImGui::End();
        return;
    }

    size_t idx = *m_selectedObjectIndex;
    if (idx >= activeScene->sceneObjects.size())
    {
        m_selectedObjectIndex.reset();
        ImGui::End();
        return;
    }

    auto& res = activeScene->sceneObjects[idx];

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
                    std::make_unique<command::RenameSceneObjectCommand>(activeScene, idx, newName)
                );
            }
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(Enter to confirm)");

        bool inTimeline = res.isInTimeline;
        if (ImGui::Checkbox("In Timeline", &inTimeline))
        {
            m_commandHistory->ExecuteCommand(
                std::make_unique<command::SetTimelineCommand>(activeScene, idx, inTimeline)
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

    demo::Scene* activeScene = GetActiveScene();
    if (!activeScene) return;

    auto& transform = activeScene->sceneObjects[index].objectTransform;

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
                std::make_unique<command::ModifyTransformCommand>(activeScene, index, newTransform)
            );
        }
        else
        {
            activeScene->sceneObjects[index].objectTransform = newTransform;
        }
    }
}

void ImGuiWindows::InspectorCameraProperties(size_t index)
{
    if (!ImGui::CollapsingHeader(ICON_MDI_CAMERA " Camera", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    demo::Scene* activeScene = GetActiveScene();
    if (!activeScene) return;

    auto* cam = std::get<graphics::resources::object::Camera*>(
        activeScene->sceneObjects[index].object);

    if (!cam) return;

    ImGui::DragFloat("FOV", &cam->fov, 0.5f, 5.0f, 170.0f);
    ImGui::DragFloat("Aspect Ratio", &cam->aspectRatio, 0.001f, 0.1f, 4.0f);
    ImGui::DragFloat("Near Clip", &cam->zNear, 0.001f, 0.001f, 10.0f);
    ImGui::DragFloat("Far Clip", &cam->zFar, 1.0f, 1.0f, 100000.0f);

    bool isActive = (activeScene->GetActiveCamera() == cam);
    if (ImGui::Checkbox("Active Camera", &isActive))
    {
        if (isActive)
            activeScene->SetActiveCamera(index);
    }
}

void ImGuiWindows::InspectorLightProperties(size_t index)
{
    if (!ImGui::CollapsingHeader(ICON_MDI_LIGHTBULB_OUTLINE " Light", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    demo::Scene* activeScene = GetActiveScene();
    if (!activeScene) return;

    auto* light = std::get<graphics::resources::Light*>(
        activeScene->sceneObjects[index].object);

    if (!light) return;

    ImGui::ColorEdit3("Color", glm::value_ptr(light->color));
    ImGui::DragFloat("Intensity", &light->intensity, 0.1f, 0.0f, 1000.0f);
    ImGui::DragFloat("Radius", &light->radius, 0.01f, 0.0f, 100.0f);
    ImGui::Checkbox("Enabled", &light->enabled);
}

void ImGuiWindows::Viewport()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport");
    ImGui::PopStyleVar();

    ImVec2 viewportPos = ImGui::GetWindowPos();
    ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
    ImVec2 avail = ImGui::GetContentRegionAvail();

    uint32_t width = std::max(1u, static_cast<uint32_t>(avail.x));
    uint32_t height = std::max(1u, static_cast<uint32_t>(avail.y));

    m_imGuiContext->DrawViewportComponent(width, height);

    const ImVec2 toolbarOrigin = ImVec2(
        viewportPos.x + contentMin.x + 8.0f,
        viewportPos.y + contentMin.y + 8.0f);

    const float  btnSize = 28.0f;
    const float  btnSpacing = 4.0f;
    const ImVec4 colActive = ImVec4(0.26f, 0.59f, 0.98f, 1.0f);
    const ImVec4 colNormal = ImVec4(0.20f, 0.20f, 0.20f, 0.80f);

    ImDrawList* dl = ImGui::GetWindowDrawList();

    float pillW = (btnSize + btnSpacing) * 4.0f + btnSpacing;
    float pillH = btnSize + btnSpacing * 2.0f;
    dl->AddRectFilled(
        ImVec2(toolbarOrigin.x - btnSpacing, toolbarOrigin.y - btnSpacing),
        ImVec2(toolbarOrigin.x - btnSpacing + pillW, toolbarOrigin.y - btnSpacing + pillH),
        IM_COL32(30, 30, 30, 180), 6.0f);

    ImGui::SetCursorScreenPos(toolbarOrigin);

    auto GizmoBtn = [&](const char* icon, bool active, const char* tooltip) -> bool
        {
            ImGui::PushStyleColor(ImGuiCol_Button, active ? colActive : colNormal);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.35f, 0.35f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, colActive);
            bool clicked = ImGui::Button(icon, ImVec2(btnSize, btnSize));
            ImGui::PopStyleColor(3);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", tooltip);
            return clicked;
        };

    if (GizmoBtn(ICON_MDI_AXIS_ARROW, m_gizmoEnabled && m_gizmoOperation == ImGuizmo::TRANSLATE, "Translate (W)"))
    {
        m_gizmoEnabled = true;
        m_gizmoOperation = ImGuizmo::TRANSLATE;
    }
    ImGui::SameLine(0, btnSpacing);

    if (GizmoBtn(ICON_MDI_ROTATE_3D, m_gizmoEnabled && m_gizmoOperation == ImGuizmo::ROTATE, "Rotate (E)"))
    {
        m_gizmoEnabled = true;
        m_gizmoOperation = ImGuizmo::ROTATE;
    }
    ImGui::SameLine(0, btnSpacing);

    if (GizmoBtn(ICON_MDI_ARROW_EXPAND_ALL, m_gizmoEnabled && m_gizmoOperation == ImGuizmo::SCALE, "Scale (R)"))
    {
        m_gizmoEnabled = true;
        m_gizmoOperation = ImGuizmo::SCALE;
    }
    ImGui::SameLine(0, btnSpacing);

    if (GizmoBtn(ICON_MDI_CURSOR_DEFAULT, !m_gizmoEnabled, "Disable gizmo (Q)"))
        m_gizmoEnabled = false;

    if (ImGui::IsWindowHovered())
    {
        if (ImGui::IsKeyPressed(ImGuiKey_W)) { m_gizmoEnabled = true;  m_gizmoOperation = ImGuizmo::TRANSLATE; }
        if (ImGui::IsKeyPressed(ImGuiKey_E)) { m_gizmoEnabled = true;  m_gizmoOperation = ImGuizmo::ROTATE; }
        if (ImGui::IsKeyPressed(ImGuiKey_R)) { m_gizmoEnabled = true;  m_gizmoOperation = ImGuizmo::SCALE; }
        if (ImGui::IsKeyPressed(ImGuiKey_Q)) { m_gizmoEnabled = false; }
    }

    demo::Scene* activeScene = GetActiveScene();

    if (m_gizmoEnabled && activeScene && m_selectedObjectIndex.has_value())
    {
        size_t idx = *m_selectedObjectIndex;
        if (idx < activeScene->sceneObjects.size())
        {
            auto* cam = activeScene->GetActiveCamera();
            if (cam)
            {
                ImGuizmo::SetOrthographic(false);
                ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());

                ImGuizmo::SetRect(
                    viewportPos.x + contentMin.x,
                    viewportPos.y + contentMin.y,
                    static_cast<float>(width),
                    static_cast<float>(height));

                glm::mat4 view = cam->GetViewMatrix();
                glm::mat4 projection = cam->GetProjectionMatrix();

                projection[1][1] *= -1.0f;

                auto& transform = activeScene->sceneObjects[idx].objectTransform;
                glm::mat4 matrix = transform.GetMatrix();

                ImGuizmo::Manipulate(
                    glm::value_ptr(view),
                    glm::value_ptr(projection),
                    m_gizmoOperation,
                    ImGuizmo::LOCAL,
                    glm::value_ptr(matrix));

                if (ImGuizmo::IsUsing())
                {
                    glm::vec3 translation, scale, skew;
                    glm::quat rotation;
                    glm::vec4 perspective;
                    glm::decompose(matrix, scale, rotation, translation, skew, perspective);

                    graphics::resources::property::Transform newTransform;
                    newTransform.SetPosition(translation);
                    newTransform.SetRotation(rotation);
                    newTransform.SetScale(scale);

                    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
                    {
                        m_commandHistory->ExecuteCommand(
                            std::make_unique<command::ModifyTransformCommand>(
                                activeScene, idx, newTransform));
                    }
                    else
                    {
                        transform = newTransform;
                    }
                }
            }
        }
    }

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
        {

        }

        if (ImGui::MenuItem("Save Project"))
        {
            if (m_scenes && !m_scenes->empty())
            {
                auto dest = pfd::save_file(
                    "Save Project", "",
                    { "YAML Project", "*.yaml" }
                ).result();

                if (!dest.empty())
                {
                    std::filesystem::path outPath(dest);
                    if (outPath.extension() != ".yaml")
                        outPath += ".yaml";

                    utils::YamlSerializer::SaveProject(outPath, *m_scenes);
                }
            }
        }

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

void ImGuiWindows::EditTransformByIndice(const float*, const float*, int) {}