#pragma once
#include <core/manager/ressourceManager.h>

#include <scene/timeline/components/mesh.h>
#include <scene/timeline/components/light.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <limits>

#include <imgui/imgui.h>
#include <memory>
#include <utility>

namespace graphics
{
    namespace ressources {
        class Mesh;
    }
    class Renderer;
}

namespace core::gpu {
    class Device;
    class Texture;
}

class ImGuiWindows;

struct Inspect
{
    Inspect(ImGuiWindows* _windowManager, RessourceManager* _ressourceManager);

    template<typename T>
    void Draw(T& object) {
        ImGui::Text("No inspector available for this type.");
    };

private:
    RessourceManager* m_ressourceManager;
    ImGuiWindows* m_windowManager;
};

template<>
void Inspect::Draw(glm::mat4& transform);

template<>
void Inspect::Draw(timeline::MeshInstance& _mesh);

template<>
void Inspect::Draw(timeline::Light& _light);