#ifndef VRAKTAL_EDITOR_SCENE_H
#define VRAKTAL_EDITOR_SCENE_H
#pragma once

#include <vector>
#include <string>
#include <optional>
#include <demo/sceneResource.h>

namespace demo
{
    class Scene
    {
    public:
        explicit Scene(std::string name) : name(std::move(name)) {}

        void Add(SceneResource resource)
        {
            if (std::holds_alternative<graphics::resources::object::Camera*>(resource.object)
                && !m_activeCameraIndex.has_value())
            {
                m_activeCameraIndex = sceneObjects.size();
            }
            sceneObjects.push_back(std::move(resource));
        }

        graphics::resources::object::Camera* GetActiveCamera() const
        {
            if (!m_activeCameraIndex.has_value())
            {
                return nullptr;
            }
            return std::get<graphics::resources::object::Camera*>(
                sceneObjects[*m_activeCameraIndex].object
            );
        }

        void SetActiveCamera(size_t index)
        {
            m_activeCameraIndex = index;
        }

        std::string                  name;
        std::vector<SceneResource>   sceneObjects;

    private:
        std::optional<size_t>        m_activeCameraIndex;
    };
}

#endif //VRAKTAL_EDITOR_SCENE_H