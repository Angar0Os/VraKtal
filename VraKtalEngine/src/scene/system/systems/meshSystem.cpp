#include <scene/system/systems/meshSystem.h>
#include <scene/timeline/components/mesh.h>
#include <graphics/renderer.h>
#include <core/manager/ressourceManager.h>
#include <scene/scene.h>

MeshSystem::MeshSystem(graphics::Renderer* _renderer, RessourceManager* _reManager) : m_ressourceManager(_reManager) , m_renderer(_renderer) {
}

MeshSystem::~MeshSystem()
{
    std::cout << "Destroying MeshSystem" << std::endl;
}

void MeshSystem::Update(Scene& _scene)
{
    // Iterate over all entities with a MeshInstance component
    auto& meshInstances = _scene.GetComponentStorage<timeline::MeshInstance>();
    for (auto& meshInstance : meshInstances.Components())
    {
        if (meshInstance.meshID != -1)
        {
            if (meshInstance.keyframes.size() == 0)
            {
                m_renderer->PushMesh(&m_ressourceManager->GetRessource<graphics::resources::Mesh>(meshInstance.meshID), meshInstance.temp_properties.transform);
            }
        }
    }
}