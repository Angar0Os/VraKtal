#pragma once
#include <imgui/imgui.h>
#include <typeindex>
#include <scene/timeline/entityBase.h>
#include <iostream>

class ImGuiWindows;
class WindowHierarchy;
class WindowInspector;

class RightClick
{
public:
    RightClick(ImGuiWindows* _windows);
	~RightClick();

    template<typename T>
    bool Draw(T* object) {
        if (!object)
            return false;

        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        { //Set Value correctly and open menu
            std::cout << "Right click on " << typeid(T).name() << std::endl;
            m_lastType = object;
            bMenuOpen = true;

            ImGui::SetNextWindowPos(ImGui::GetMousePos());
            ImGui::OpenPopup("RightClickPopup");
        }

        if (m_lastType == object && ImGui::BeginPopup("RightClickPopup")) //Draw menu
        {
            Content<T>(object);
            ImGui::EndPopup();
            return true;
        }
        else if (m_lastType == object && bMenuOpen)
        {
            CloseMenu();
            return false;
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

#pragma region Hierarchy
template<>
void RightClick::Content(WindowHierarchy* _window);
template<>
void RightClick::Content(EntityID* _ID);
#pragma endregion

#pragma region Inspector
template<>
void RightClick::Content(WindowInspector* _window);
#pragma endregion