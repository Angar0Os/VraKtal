#pragma once

#include <utils/NamedStorageMapped.h>
#include <loaders/loaderBase.h>

#include <string>
#include <typeindex>
#include <unordered_map>

/*
	Asset = ce qu'ou sauvegardes dans le projet
	Resource = ce qu'on crées pour le runtime
	Instance = ressource runtime modifiable

	Donc Cette classe ne va s'occuper que des Ressources
	{
		Mesh
		Texture
		MaterialInstance
		...
	}

	Les instances ce serait plutot la geometrie (timeline::Mesh->assetID)
*/


namespace core::gpu {
	class Device;
}

namespace graphics::assets {
	struct Material;
}

class AssetManager 
{
public:
	AssetManager();
	~AssetManager();

	template<typename T>
	T& LoadAsset(std::string _path , const loaders::LoadOptions* options);
	
	template<typename T>
	T& GetAsset(std::string _path);
	
	template<typename T>
	T& GetAsset(uint32_t _ID);
	
	template<typename T> 
	uint32_t GetAssetID(std::string _path);

	template<typename T> 
	const std::string& GetAssetPath(uint32_t _ID);

	template<typename T> 
	const std::string& GetAssetPath(T& _ressource);
	
	template<typename T> 
	uint32_t AddExistingAsset(std::string _path , T& _ressource);

	std::unordered_map<std::type_index, InterfaceStorage*>& GetAssetStorages() { return m_storages; };

private:
	std::unordered_map<std::type_index, loaders::LoaderBase*> m_loaders;
	std::unordered_map<std::type_index, InterfaceStorage*> m_storages;

	template<typename TLoader, typename TRessource>
	void RegisterType(loaders::LoaderBase* _loader);

	core::gpu::Device* m_device = nullptr;
};

template<typename T>
inline T& AssetManager::LoadAsset(std::string _path , const loaders::LoadOptions* options)
{
	std::replace(_path.begin(), _path.end(), '\\', '/');

	NamedStorageMap<T>& storage= static_cast<Storage<T>*>(m_storages[typeid(T)])->data;
	if (storage.Contains(_path))
	{
#ifdef VRAKTAL_EDITOR
		std::cout << "ressource at: " << _path << "already loaded" << std::endl;
#endif // VRAKTAL_EDITOR
		return GetAsset<T>(_path);
	}

	loaders::LoaderBase* loader = m_loaders[typeid(T)];
	std::shared_ptr<T> loaded = std::static_pointer_cast<T>(loader->Load(_path , options));

	storage.Add(_path);
	auto id = storage.Find(_path);
	storage.Get(id) = std::move(*loaded);

	return storage.Get(id);
}

template<typename T>
inline T& AssetManager::GetAsset(std::string _path)
{
	std::replace(_path.begin(), _path.end(), '\\', '/');

	NamedStorageMap<T>& storage = static_cast<Storage<T>*>(m_storages[typeid(T)])->data;
	return storage.Get(storage.Find(_path));
}

template<typename T>
inline T& AssetManager::GetAsset(uint32_t _ID)
{
	NamedStorageMap<T>& storage = static_cast<Storage<T>*>(m_storages[typeid(T)])->data;
	return storage.Get(_ID);
}

template<typename T>
inline uint32_t AssetManager::GetAssetID(std::string _path)
{
	std::replace(_path.begin(), _path.end(), '\\', '/');
	NamedStorageMap<T>& storage = static_cast<Storage<T>*>(m_storages[typeid(T)])->data;
	
	return storage.Find(_path);
}

template<typename T>
inline const std::string& AssetManager::GetAssetPath(uint32_t _ID)
{
	NamedStorageMap<T>& storage = static_cast<Storage<T>*>(m_storages[typeid(T)])->data;
	return storage.GetName(_ID);
}

template<typename T>
inline const std::string& AssetManager::GetAssetPath(T& _ressource) // cette fonction est couteuse
{
	NamedStorageMap<T>& storage = static_cast<Storage<T>*>(m_storages[typeid(T)])->data;
	return storage.GetName(storage.Find(_ressource));
}

template<typename T>
inline uint32_t AssetManager::AddExistingAsset(std::string _path, T& _ressource)
{
	std::replace(_path.begin(), _path.end(), '\\', '/');
	NamedStorageMap<T>& storage = static_cast<Storage<T>*>(m_storages[typeid(T)])->data;
	if (storage.Contains(_path))
	{
#ifdef VRAKTAL_EDITOR
		std::cout << "ressource at: " << _path << "already loaded" << std::endl;
#endif // VRAKTAL_EDITOR
		return storage.Find(_path);
	}

	storage.Add(_path);
	auto id = storage.Find(_path);
	_ressource.id = id;
	storage.Get(id) = std::move(_ressource);
	std::cout << "Added asset at "<< _path << " with id: "<< id << std::endl;

	return id;
}

template<typename TLoader , typename TRessource>
inline void AssetManager::RegisterType(loaders::LoaderBase* _loader)
{
	m_loaders[typeid(TRessource)] = _loader;
	m_storages[typeid(TRessource)] = new Storage<TRessource>;
}