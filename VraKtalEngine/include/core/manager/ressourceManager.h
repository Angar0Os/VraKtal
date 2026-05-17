#pragma once
#include <typeindex>
#include <unordered_map>
#include <memory>

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

namespace graphics
{
	class Renderer;
}

VRAKTAL_FORWARD_DECLARE_ASSET_RESOURCE_FACTORY(Material, MaterialFactory)
VRAKTAL_FORWARD_DECLARE_ASSET_RESOURCE_FACTORY(Mesh, MeshFactory)
VRAKTAL_FORWARD_DECLARE_ASSET_RESOURCE_FACTORY(Texture, TextureFactory)
#pragma endregion

#pragma region RessourceTraits
template<typename TResource>
struct ResourceTraits;
VRAKTAL_RESOURCE_TRAITS(Mesh, MeshFactory)
VRAKTAL_RESOURCE_TRAITS(Texture, TextureFactory)
VRAKTAL_RESOURCE_TRAITS(Material, MaterialFactory);
#pragma endregion

class RessourceManager
{
public:
	RessourceManager(core::gpu::Device& _device , graphics::Renderer& _renderer);
	~RessourceManager();

	template<typename TResource , typename... Args>
	void RegisterRessourceType(Args&&... args);

	template<typename TResource>
	uint32_t CreateRessource(const ResourceTraits<TResource>::AssetType& _asset);

	template<typename TResource>
	TResource* CreateRessource_ptr(const ResourceTraits<TResource>::AssetType& _asset);

	template<typename TResource>
	TResource& GetResource(uint32_t _assetID);
	
	template<typename TResource>
	bool HasResource(uint32_t _assetID);

private:
	std::vector<std::unique_ptr<IFactoryWrapper>> m_factories;
	std::vector<std::unique_ptr<IRessourceStorage>> m_resourcesStorages;

	core::gpu::Device& m_device;
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

    std::cout << "Registered resource type: " << typeid(TResource).name() << std::endl;
}

template<typename TResource>
inline uint32_t RessourceManager::CreateRessource(
	const typename ResourceTraits<TResource>::AssetType& _asset
)
{
	using TFactory = typename ResourceTraits<TResource>::FactoryType;

	const auto id = ComponentTypeID<TResource>();

	if (_asset.id == 0xFFFFFFFFu)
		throw std::runtime_error("CreateRessource: asset id is INVALID_ID");

	if (id >= m_factories.size())
		throw std::runtime_error("CreateRessource: factory vector too small");

	if (id >= m_resourcesStorages.size())
		throw std::runtime_error("CreateRessource: storage vector too small");

	if (!m_factories[id])
		throw std::runtime_error("CreateRessource: factory not registered");

	if (!m_resourcesStorages[id])
		throw std::runtime_error("CreateRessource: storage not registered");

	auto* storage =
		static_cast<ResourceStorageData<TResource>*>(m_resourcesStorages[id].get());

	auto* factory =
		static_cast<FactoryWrapper<TFactory>*>(m_factories[id].get());

	TResource resource = factory->factory.Create(_asset);

	storage->data.Add(_asset.id, std::move(resource));

	std::cout << "Ressource created for asset: " << _asset.id << std::endl;

	return _asset.id;
}

template<typename TResource>
inline TResource* RessourceManager::CreateRessource_ptr(
	const typename ResourceTraits<TResource>::AssetType& _asset
)
{
	using TFactory = typename ResourceTraits<TResource>::FactoryType;

	const auto id = ComponentTypeID<TResource>();

	std::cout << "CreateRessource<" << typeid(TResource).name() << ">\n";
	std::cout << "type id: " << id << "\n";
	std::cout << "factories size: " << m_factories.size() << "\n";
	std::cout << "storages size: " << m_resourcesStorages.size() << "\n";
	std::cout << "asset id: " << _asset.id << "\n";

	if (_asset.id == 0xFFFFFFFFu)
		throw std::runtime_error("CreateRessource: asset id is INVALID_ID");

	if (id >= m_factories.size())
		throw std::runtime_error("CreateRessource: factory vector too small");

	if (id >= m_resourcesStorages.size())
		throw std::runtime_error("CreateRessource: storage vector too small");

	if (!m_factories[id])
		throw std::runtime_error("CreateRessource: factory not registered");

	if (!m_resourcesStorages[id])
		throw std::runtime_error("CreateRessource: storage not registered");

	auto* storage =
		static_cast<ResourceStorageData<TResource>*>(m_resourcesStorages[id].get());

	auto* factory =
		static_cast<FactoryWrapper<TFactory>*>(m_factories[id].get());

	TResource resource = factory->factory.Create(_asset);

	storage->data.Add(_asset.id, std::move(resource));

	std::cout << "Ressource created for asset: " << _asset.id << std::endl;

	return &GetResource<TResource>(_asset.id);
}

template<typename TResource>
inline TResource& RessourceManager::GetResource(uint32_t _assetID)
{
	const auto id = ComponentTypeID<TResource>();
	return static_cast<ResourceStorageData<TResource>*>(m_resourcesStorages[id].get())->data.Get(_assetID);
}

template<typename TResource>
inline bool RessourceManager::HasResource(uint32_t _assetID)
{
	const auto id = ComponentTypeID<TResource>();
	return static_cast<ResourceStorageData<TResource>*>(m_resourcesStorages[id].get())->data.Has(_assetID);
}