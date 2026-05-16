#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <scene/scene.h>
#include <utils/denseStorage.h>

class SceneManager
{
public:
    SceneManager();
    ~SceneManager();

    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    SceneManager(SceneManager&&) noexcept = default;
    SceneManager& operator=(SceneManager&&) noexcept = default;

    EntityID CreateScene(std::string _name);
    EntityID AddScene(std::string _name, Scene& _scene);
    EntityID AddScene(std::string _name, Scene&& _scene);

    bool IsValidScene(EntityID _id) const;

    Scene& GetScene(EntityID _id);
    const Scene& GetScene(EntityID _id) const;

    const std::string& GetSceneName(EntityID _id) const;


    EntityID GetSceneID(const std::string& _name) const;

    void RemoveScene(const std::string& _name);
    void RemoveScene(EntityID _id);

    void SetSceneName(EntityID _id, const std::string& _name);
    void SetSceneName(const std::string& _currentName, const std::string& _newName);

    void SetActiveScene(EntityID _id, bool _bActive);
    void SetActiveScene(const std::string& _name, bool _bActive);

    bool IsSceneActive(EntityID _id) const;
    const std::vector<EntityID>& GetActiveSceneIDs() const;

    EntityID GetFirstActiveSceneID() const;

    std::vector<Scene>& GetScenes();
    const std::vector<Scene>& GetScenes() const;

private:
    EntityID AllocateSceneID();
    void ReleaseSceneID(EntityID _id);
    void RemoveSceneFromActiveList(EntityID _id);

private:
    DenseStorage<EntityID, Scene> m_scenes;

    std::unordered_map<std::string, EntityID> m_nameToId;
    std::unordered_map<EntityID, std::string> m_idToName;

    std::vector<EntityID> m_activeScenes;
    std::vector<EntityID> m_freeSceneIDs;
    EntityID m_nextSceneID = 0;
};
