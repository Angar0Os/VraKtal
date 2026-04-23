#pragma once

#include <utils/NamedStorageMapped.h>
#include <loaders/loaderBase.h>

#include <string>
#include <typeindex>
#include <unordered_map>

namespace core::gpu {
	class Device;
}

struct InterfaceStorage {
	virtual ~InterfaceStorage() = default;
};

template<typename T>
struct Storage : InterfaceStorage
{
	NamedStorageMap<T> data;;
};

class RessourceManager
{
public:
	RessourceManager(core::gpu::Device* _device);
	~RessourceManager();

	template<typename T>
	T* LoadRessource(std::string _path);
	
	template<typename T>
	T& GetRessource(std::string _path);
	
	template<typename T>
	T& GetRessource(uint32_t _ID);
	
	template<typename T> 
	uint32_t GetRessourceID(std::string _path);

	template<typename T> 
	const std::string& GetRessourcePath(uint32_t _ID);

	template<typename T> 
	const std::string& GetRessourcePath(T& _ressource);
	

private:
	std::unordered_map<std::type_index, LoaderBase*> m_loaders;
	std::unordered_map<std::type_index, InterfaceStorage*> m_storages;

	template<typename TLoader, typename TRessource>
	void RegisterType(LoaderBase* _loader);

	template<typename TRessource>
	void Clear();

	core::gpu::Device* m_device = nullptr;
};

template<typename T>
inline T* RessourceManager::LoadRessource(std::string _path)
{
	LoaderBase* loader = m_loaders[typeid(T)];
	NamedStorageMap<T>& storage= static_cast<Storage<T>*>(m_storages[typeid(T)])->data;

	std::shared_ptr<T> loaded = std::static_pointer_cast<T>(loader->Load(_path));

	storage.Add(_path);
	auto id = storage.Find(_path);
	storage.Get(id) = std::move(*loaded);

	return &storage.Get(id);
}

template<typename T>
inline T& RessourceManager::GetRessource(std::string _path)
{
	NamedStorageMap<T>& storage = static_cast<Storage<T>*>(m_storages[typeid(T)])->data;
	return storage.Get(storage.Find(_path));
}

template<typename T>
inline T& RessourceManager::GetRessource(uint32_t _ID)
{
	NamedStorageMap<T>& storage = static_cast<Storage<T>*>(m_storages[typeid(T)])->data;
	return storage.Get(_ID);
}

template<typename T>
inline uint32_t RessourceManager::GetRessourceID(std::string _path)
{
	NamedStorageMap<T>& storage = static_cast<Storage<T>*>(m_storages[typeid(T)])->data;
	return storage.Find(_path);
}

template<typename T>
inline const std::string& RessourceManager::GetRessourcePath(uint32_t _ID)
{
	NamedStorageMap<T>& storage = static_cast<Storage<T>*>(m_storages[typeid(T)])->data;
	return storage.GetName(_ID);
}

template<typename T>
inline const std::string& RessourceManager::GetRessourcePath(T& _ressource) // cette fonction est couteuse niquez moi
{
	NamedStorageMap<T>& storage = static_cast<Storage<T>*>(m_storages[typeid(T)])->data;
	return storage.GetName(storage.Find(_ressource));
}

template<typename TLoader , typename TRessource>
inline void RessourceManager::RegisterType(LoaderBase* _loader)
{
	m_loaders[typeid(TRessource)] = _loader;
	m_storages[typeid(TRessource)] = new Storage<TRessource>;
}

template<typename TRessource>
inline void RessourceManager::Clear()
{
}