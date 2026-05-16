#include <core/manager/sceneManager.h>

#include <stdexcept>
#include <utility>

SceneManager::SceneManager()
{
}

SceneManager::~SceneManager()
{
}

EntityID SceneManager::CreateScene(std::string _name)
{
    auto it = m_nameToId.find(_name);
    if (it != m_nameToId.end())
        return it->second;

    EntityID id = AllocateSceneID();

    Scene scene(_name, true);
    scene.SetActive(true);

    m_scenes.Add(id, std::move(scene));
    m_nameToId.emplace(_name, id);
    m_idToName.emplace(id, _name);

    SetActiveScene(id, true);

    return id;
}

EntityID SceneManager::AddScene(std::string _name, Scene& _scene)
{
    return AddScene(std::move(_name), std::move(_scene));
}

EntityID SceneManager::AddScene(std::string _name, Scene&& _scene)
{
    auto it = m_nameToId.find(_name);
    EntityID id = INVALID_ENTITY;

    if (it == m_nameToId.end())
    {
        id = AllocateSceneID();
        m_nameToId.emplace(_name, id);
        m_idToName.emplace(id, _name);
    }
    else
    {
        id = it->second;
    }

    _scene.SetName(_name);
    _scene.SetActive(true);

    m_scenes.Add(id, std::move(_scene));
    SetActiveScene(id, true);

    return id;
}

bool SceneManager::IsValidScene(EntityID _id) const
{
    return _id != INVALID_ENTITY && m_scenes.Has(_id);
}

Scene& SceneManager::GetScene(EntityID _id)
{
    return m_scenes.Get(_id);
}

const Scene& SceneManager::GetScene(EntityID _id) const
{
    return m_scenes.Get(_id);
}

EntityID SceneManager::GetSceneID(const std::string& _name) const
{
    auto it = m_nameToId.find(_name);
    if (it == m_nameToId.end())
        return INVALID_ENTITY;

    return it->second;
}

const std::string& SceneManager::GetSceneName(EntityID _id) const
{
    auto it = m_idToName.find(_id);

    if (it == m_idToName.end())
        throw std::runtime_error("SceneManager::GetSceneName(): invalid scene id");

    return it->second;
}

void SceneManager::RemoveScene(const std::string& _name)
{
    EntityID id = GetSceneID(_name);
    if (id == INVALID_ENTITY)
        return;

    RemoveScene(id);
}

void SceneManager::RemoveScene(EntityID _id)
{
    if (!IsValidScene(_id))
        return;

    SetActiveScene(_id, false);

    auto nameIt = m_idToName.find(_id);
    if (nameIt != m_idToName.end())
    {
        m_nameToId.erase(nameIt->second);
        m_idToName.erase(nameIt);
    }

    m_scenes.Remove(_id);
    ReleaseSceneID(_id);
}

void SceneManager::SetSceneName(EntityID _id, const std::string& _name)
{
    if (!IsValidScene(_id))
        return;

    auto existing = m_nameToId.find(_name);
    if (existing != m_nameToId.end() && existing->second != _id)
        throw std::runtime_error("SceneManager::SetSceneName(): name already used");

    auto oldNameIt = m_idToName.find(_id);
    if (oldNameIt != m_idToName.end())
        m_nameToId.erase(oldNameIt->second);

    m_nameToId[_name] = _id;
    m_idToName[_id] = _name;
    m_scenes.Get(_id).SetName(_name);
}

void SceneManager::SetSceneName(const std::string& _currentName, const std::string& _newName)
{
    EntityID id = GetSceneID(_currentName);
    if (id == INVALID_ENTITY)
        return;

    SetSceneName(id, _newName);
}

void SceneManager::SetActiveScene(EntityID _id, bool _bActive)
{
    if (!IsValidScene(_id))
        return;

    Scene& scene = m_scenes.Get(_id);
    scene.SetActive(_bActive);

    auto it = std::find(m_activeScenes.begin(), m_activeScenes.end(), _id);

    if (_bActive)
    {
        if (it == m_activeScenes.end())
            m_activeScenes.push_back(_id);
    }
    else
    {
        if (it != m_activeScenes.end())
            m_activeScenes.erase(it);
    }
}

void SceneManager::SetActiveScene(const std::string& _name, bool _bActive)
{
    EntityID id = GetSceneID(_name);
    if (id == INVALID_ENTITY)
        return;

    SetActiveScene(id, _bActive);
}

bool SceneManager::IsSceneActive(EntityID _id) const
{
    return std::find(m_activeScenes.begin(), m_activeScenes.end(), _id) != m_activeScenes.end();
}

const std::vector<EntityID>& SceneManager::GetActiveSceneIDs() const
{
    return m_activeScenes;
}

EntityID SceneManager::GetFirstActiveSceneID() const
{
    if (m_activeScenes.empty())
        return INVALID_ENTITY;

    return m_activeScenes.front();
}

std::vector<Scene>& SceneManager::GetScenes()
{
    return m_scenes.Values();
}

const std::vector<Scene>& SceneManager::GetScenes() const
{
    return m_scenes.Values();
}

EntityID SceneManager::AllocateSceneID()
{
    if (!m_freeSceneIDs.empty())
    {
        EntityID id = m_freeSceneIDs.back();
        m_freeSceneIDs.pop_back();
        return id;
    }

    return m_nextSceneID++;
}

void SceneManager::ReleaseSceneID(EntityID _id)
{
    if (_id == INVALID_ENTITY)
        return;

    if (std::find(m_freeSceneIDs.begin(), m_freeSceneIDs.end(), _id) == m_freeSceneIDs.end())
        m_freeSceneIDs.push_back(_id);
}

void SceneManager::RemoveSceneFromActiveList(EntityID _id)
{
    auto it = std::find(m_activeScenes.begin(), m_activeScenes.end(), _id);
    if (it != m_activeScenes.end())
        m_activeScenes.erase(it);
}
