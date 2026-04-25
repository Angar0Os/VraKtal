#include <core/manager/ressourceManager.h>

#include <core/gpu/device.h>

#include <graphics/resources/object/mesh.h>
#include <loaders/meshLoader.h>
#include <core/gpu/texture.h>

RessourceManager::RessourceManager(core::gpu::Device* _device)
{
	m_device = _device;
	RegisterType<loaders::MeshLoader, graphics::resources::Mesh>(new loaders::MeshLoader(_device));
	// RegisterType<loaders::TextureLoader, core::gpu::Texture>(new loaders::TextureLoader(*_device)); not used ; - ;


}

RessourceManager::~RessourceManager()
{
	for (auto& var : m_loaders)
	{
		delete var.second;
	}

	for (auto& var : m_storages)
	{
		delete var.second;
	}
}
