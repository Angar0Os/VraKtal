#include <core/manager/assetManager.h>

#include <loaders/meshLoader.h>
#include <graphics/assets/mesh.h>

#include <graphics/assets/material.h>

AssetManager::AssetManager()
{
	//rien nest bon j'ai envie de canner

	/*
		On utilise les assets avec la factory
		La factory retourne une ressource on stoque les ressources dans le ressourceManager
		Le managerAsset et le ressourceManager sont interconnectee
		
		La factory Creer les instances aussi
	*/

	RegisterType<loaders::MeshLoader, graphics::assets::Mesh>(new loaders::MeshLoader());
	RegisterType<void*, graphics::assets::Material>(nullptr);
}

AssetManager::~AssetManager()
{
	for (auto& var : m_loaders)
	{
		delete var.second;
		var.second = nullptr;
	}

	for (auto& var : m_storages)
	{
		delete var.second;
		var.second = nullptr;
	}
}