#pragma once
#include <typeindex>
#include <unordered_map>
#include <memory>

#include <graphics/assets/mesh.h>
#include <graphics/assets/material.h>
#include <graphics/resources/object/mesh.h>

#include <utils/NamedStorageMapped.h>
#include <utils/denseStorage.h>
#include <utils/typeIndex.h>

#include <utils/macro.h>

struct IFactoryWrapper
{
	virtual ~IFactoryWrapper() = default;
};

template<typename TFactory>
struct FactoryWrapper : IFactoryWrapper
{
	template<typename... Args>
	FactoryWrapper(Args&&... args) : factory(std::forward<Args>(args)...) {}
	TFactory factory;
};

struct IRessourceStorage
{
	virtual ~IRessourceStorage() = default;
};

template<typename TResource>
struct ResourceStorageData : public IRessourceStorage
{
	DenseStorage<uint32_t, TResource> data;
};

#pragma region forwardDecl
class AssetManager;
namespace core::gpu 
{
	class Device;
}

VRAKTAL_FORWARD_DECLARE_ASSET_RESOURCE_FACTORY(Material, MaterialFactory)
VRAKTAL_FORWARD_DECLARE_ASSET_RESOURCE_FACTORY(Mesh, MeshFactory)
VRAKTAL_FORWARD_DECLARE_ASSET_RESOURCE_FACTORY(Texture, TextureFactory)
#pragma endregion

#pragma region RessourceTraits
template<typename TResource>
struct ResourceTraits;
VRAKTAL_RESOURCE_TRAITS(Mesh)
VRAKTAL_RESOURCE_TRAITS(Texture)
VRAKTAL_RESOURCE_TRAITS(Material)
#pragma endregion

class RessourceManager
{
public:
	RessourceManager(core::gpu::Device& _device , AssetManager& _astManager);
	~RessourceManager();

	template<typename TResource , typename... Args>
	void RegisterRessourceType(Args&&... args);

	template<typename TResource>
	uint32_t CreateRessource(const ResourceTraits<TResource>::AssetType& _asset);

	template<typename TResource>
	TResource& GetResource(uint32_t _assetID);

private:
	std::vector<std::unique_ptr<IFactoryWrapper>> m_factories;
	std::vector<std::unique_ptr<IRessourceStorage>> m_resourcesStorages;

	core::gpu::Device& m_device;
	AssetManager& m_assetManager;
};

template<typename TResource, typename... Args>
void RessourceManager::RegisterRessourceType(Args&&... args)
{
	using TFactory = typename ResourceTraits<TResource>::FactoryType;
	const auto id = ComponentTypeID<TResource>();
	if (m_factories.size() <= id)
	{
		m_factories.resize(id + 1);
	}
	if (!m_factories[id])
	{
		m_factories[id] = std::make_unique<FactoryWrapper<TFactory>>(std::forward<Args>(args)...);
	}
	if (m_resourcesStorages.size() <= id)
	{
		m_resourcesStorages.resize(id + 1);
	}
	if (!m_resourcesStorages[id])
	{
		m_resourcesStorages[id] = std::make_unique<ResourceStorageData<TResource>>();
	}
}

template<typename TResource>
inline uint32_t RessourceManager::CreateRessource(const ResourceTraits<TResource>::AssetType& _asset)
{
	using TFactory = typename ResourceTraits<TResource>::FactoryType;
	const auto id = ComponentTypeID<TResource>();
	static_cast<ResourceStorageData<TResource>*>(m_resourcesStorages[id].get())->data.Add(
		_asset.id,
		static_cast<FactoryWrapper<TFactory>*>(m_factories[id].get())->factory.Create(_asset));
	
	std::cout << "Ressource created for asset: " << _asset.id << std::endl;

	auto& niquezmoi = GetResource<TResource>(_asset.id);
	return _asset.id;
}

template<typename TResource>
inline TResource& RessourceManager::GetResource(uint32_t _assetID)
{
	const auto id = ComponentTypeID<TResource>();
	return static_cast<ResourceStorageData<TResource>*>(m_resourcesStorages[id].get())->data.Get(_assetID);
}