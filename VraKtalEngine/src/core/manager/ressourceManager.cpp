#include <core/manager/ressourceManager.h>

#include <core/gpu/device.h>

#include <graphics/resources/object/mesh.h>
#include <graphics/materialInstance.h>

#include <loaders/meshLoader.h>
#include <loaders/materialLoader.h>

#include <core/gpu/texture.h>

RessourceManager::RessourceManager(core::gpu::Device* _device)
{
	m_device = _device;
	RegisterType<loaders::MeshLoader, graphics::resources::Mesh>(new loaders::MeshLoader(_device));
	// RegisterType<loaders::TextureLoader, core::gpu::Texture>(new loaders::TextureLoader(*_device)); not used ; - ;
	//RegisterType<loaders::MaterialLoader, graphics::resources::MaterialInstance>(new loaders::MaterialLoader(_device));

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
