#ifndef EDITOR_SCENE_COMMANDS_H
#define EDITOR_SCENE_COMMANDS_H
#pragma once

#include "command/command.h"
#include <demo/scene.h>
#include <demo/sceneResource.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>

namespace command
{
    class AddSceneObjectCommand : public Command
    {
    public:
        AddSceneObjectCommand(demo::Scene* scene, demo::SceneResource resource)
            : m_scene(scene), m_resource(std::move(resource))
        {
        }

        bool CanExecute() const override { return m_scene != nullptr; }

        void Execute() override
        {
            m_insertedIndex = m_scene->sceneObjects.size();
            m_scene->sceneObjects.push_back(m_resource);
        }

        void Undo() override
        {
            if (m_insertedIndex.has_value() && *m_insertedIndex < m_scene->sceneObjects.size())
            {
                m_scene->sceneObjects.erase(m_scene->sceneObjects.begin() + *m_insertedIndex);
            }
        }

        std::string GetDescription() const override
        {
            return "Add object \"" + m_resource.objectName + "\"";
        }

    private:
        demo::Scene* m_scene;
        demo::SceneResource             m_resource;
        std::optional<size_t>           m_insertedIndex;
    };

    class RemoveSceneObjectCommand : public Command
    {
    public:
        RemoveSceneObjectCommand(demo::Scene* scene, size_t index)
            : m_scene(scene), m_index(index)
        {
        }

        bool CanExecute() const override
        {
            return m_scene != nullptr && m_index < m_scene->sceneObjects.size();
        }

        void Execute() override
        {
            m_removedResource = m_scene->sceneObjects[m_index];
            m_scene->sceneObjects.erase(m_scene->sceneObjects.begin() + m_index);
        }

        void Undo() override
        {
            if (m_removedResource.has_value())
            {
                m_scene->sceneObjects.insert(
                    m_scene->sceneObjects.begin() + m_index,
                    *m_removedResource
                );
            }
        }

        std::string GetDescription() const override
        {
            if (m_removedResource.has_value())
                return "Remove object \"" + m_removedResource->objectName + "\"";
            return "Remove object";
        }

    private:
        demo::Scene* m_scene;
        size_t                                  m_index;
        std::optional<demo::SceneResource>      m_removedResource;
    };

    class RenameSceneObjectCommand : public Command
    {
    public:
        RenameSceneObjectCommand(demo::Scene* scene, size_t index, std::string newName)
            : m_scene(scene), m_index(index), m_newName(std::move(newName))
        {
        }

        bool CanExecute() const override
        {
            return m_scene != nullptr && m_index < m_scene->sceneObjects.size();
        }

        void Execute() override
        {
            m_oldName = m_scene->sceneObjects[m_index].objectName;
            m_scene->sceneObjects[m_index].objectName = m_newName;
        }

        void Undo() override
        {
            if (!m_oldName.empty())
                m_scene->sceneObjects[m_index].objectName = m_oldName;
        }

        std::string GetDescription() const override
        {
            return "Rename \"" + m_oldName + "\" to \"" + m_newName + "\"";
        }

    private:
        demo::Scene* m_scene;
        size_t          m_index;
        std::string     m_newName;
        std::string     m_oldName;
    };

    class ModifyTransformCommand : public Command
    {
    public:
        ModifyTransformCommand(demo::Scene* scene, size_t index,
            const graphics::resources::property::Transform& newTransform)
            : m_scene(scene), m_index(index), m_newTransform(newTransform)
        {
        }

        bool CanExecute() const override
        {
            return m_scene != nullptr && m_index < m_scene->sceneObjects.size();
        }

        void Execute() override
        {
            m_oldTransform = m_scene->sceneObjects[m_index].objectTransform;
            m_scene->sceneObjects[m_index].objectTransform = m_newTransform;
        }

        void Undo() override
        {
            if (m_oldTransform.has_value())
                m_scene->sceneObjects[m_index].objectTransform = *m_oldTransform;
        }

        std::string GetDescription() const override
        {
            return "Transform \"" + m_scene->sceneObjects[m_index].objectName + "\"";
        }

    private:
        demo::Scene* m_scene;
        size_t                                                  m_index;
        graphics::resources::property::Transform               m_newTransform;
        std::optional<graphics::resources::property::Transform> m_oldTransform;
    };

    class SetTimelineCommand : public Command
    {
    public:
        SetTimelineCommand(demo::Scene* scene, size_t index, bool value)
            : m_scene(scene), m_index(index), m_newValue(value)
        {
        }

        bool CanExecute() const override
        {
            return m_scene != nullptr && m_index < m_scene->sceneObjects.size();
        }

        void Execute() override
        {
            m_oldValue = m_scene->sceneObjects[m_index].isInTimeline;
            m_scene->sceneObjects[m_index].isInTimeline = m_newValue;
        }

        void Undo() override
        {
            m_scene->sceneObjects[m_index].isInTimeline = m_oldValue;
        }

        std::string GetDescription() const override
        {
            return std::string(m_newValue ? "Add" : "Remove") + " \""
                + m_scene->sceneObjects[m_index].objectName + "\" from timeline";
        }

    private:
        demo::Scene* m_scene;
        size_t          m_index;
        bool            m_newValue;
        bool            m_oldValue = false;
    };

} 

#endif // EDITOR_SCENE_COMMANDS_H