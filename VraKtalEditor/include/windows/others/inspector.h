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


struct Inspect
{
    Inspect(RessourceManager* _ressourceManager) : m_ressourceManager(_ressourceManager) {};

    template<typename T>
    void Draw(T& object) {
        ImGui::Text("No inspector available for this type.");
    };

private:
    RessourceManager* m_ressourceManager;
};

template<>
void Inspect::Draw(glm::mat4& transform);

template<>
void Inspect::Draw(timeline::MeshInstance& _mesh);

template<>
void Inspect::Draw(timeline::Light& _light);