#include <scene/system/systems/meshSystem.h>
#include <scene/scene.h>
#include <scene/timeline/entities/mesh.h>
#include <graphics/renderer.h>
#include <core/gpu/buffer.h>

MeshSystem::MeshSystem(graphics::Renderer* _renderer) : m_renderer(_renderer){}

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
        if (meshInstance.mesh)
        {
            m_renderer->PushMesh(meshInstance.mesh, meshInstance.temp_transform);
        }
    }
}