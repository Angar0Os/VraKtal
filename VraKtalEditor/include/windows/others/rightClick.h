#pragma once
#include <imgui/imgui.h>
#include <typeindex>
#include <scene/timeline/entityBase.h>

class ImGuiWindows;
class WindowHierarchy;

class RightClick
{
public:
    RightClick(ImGuiWindows* _windows);
	~RightClick();

    template<typename T>
    void Draw(T* object) {
        if (!object)
            return;

        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            m_lastType = object;
            bMenuOpen = true;

            ImGui::SetNextWindowPos(ImGui::GetMousePos());
            ImGui::OpenPopup("RightClickPopup");
        }

        if (m_lastType == object && ImGui::BeginPopup("RightClickPopup"))
        {
            Content<T>(object);
            ImGui::EndPopup();
        }
        else if (m_lastType == object && bMenuOpen)
        {
            CloseMenu();
        }
    };

    template<typename T>
    void Content(T* object) {
        ImGui::Text("No inspector available for this type.");
    };

private:
	ImGuiWindows* m_windows;
    void* m_lastType = nullptr;
    bool bMenuOpen = false;

    void ResetLastType() { m_lastType = nullptr; };
    void CloseMenu() {
        bMenuOpen = false;
        ResetLastType();
    }
};

template<>
void RightClick::Content(WindowHierarchy* _window);

template<>
void RightClick::Content(EntityID* _ID);