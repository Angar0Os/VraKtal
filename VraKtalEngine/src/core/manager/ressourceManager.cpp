#include <core/manager/ressourceManager.h>
#include <factory/meshFactory.h>
#include <graphics/resources/object/mesh.h>

RessourceManager::RessourceManager(core::gpu::Device& _device , AssetManager& _astManager) : m_device(_device) , m_assetManager(_astManager)
{
	RegisterRessourceType<graphics::resources::Mesh>(_device);
}

RessourceManager::~RessourceManager()
{
}