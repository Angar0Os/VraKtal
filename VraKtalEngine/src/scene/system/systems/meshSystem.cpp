#include <scene/system/systems/meshSystem.h>
#include <scene/timeline/components/mesh.h>
#include <graphics/renderer.h>
#include <core/manager/ressourceManager.h>
#include <scene/scene.h>
#include <graphics/resources/object/mesh.h>

MeshSystem::MeshSystem(graphics::Renderer& _renderer, RessourceManager& _reManager) : m_ressourceManager(_reManager) , m_renderer(_renderer) {
}

MeshSystem::~MeshSystem()
{
    std::cout << "Destroying MeshSystem" << std::endl;
}

void MeshSystem::Update(Scene& _scene)
{
    // Iterate over all entities with a MeshInstance component
    auto& meshInstances = _scene.GetComponentStorage<timeline::MeshInstance>();
    for (auto& meshInstance : meshInstances.Values())
    {
        if (meshInstance.assetID != -1)
        {
            if (meshInstance.keyframes.size() == 0)
            {
                // TODO
                m_renderer.PushMesh(&m_ressourceManager.GetResource<graphics::resources::Mesh>(meshInstance.assetID), meshInstance.temp_properties.transform);
            }
        }
    }
}