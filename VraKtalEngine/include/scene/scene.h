#pragma once
#include <iostream>
#include <tuple>
#include <utility>
#include <type_traits>
#include <typeindex>

#include <utils/denseStorage.h>
#include <utils/typeIndex.h>
#include "timeline/entityBase.h"
#include <unordered_map>
#include <memory>

struct IComponentStorage
{
	virtual ~IComponentStorage() = default;

	virtual void Remove(EntityID entity) = 0;
	virtual bool Has(EntityID entity) const = 0;
	virtual size_t Size() const = 0;
};

template<typename TComponent>
struct ComponentStorageData : public IComponentStorage
{
	DenseStorage<EntityID, TComponent> data;

	void Remove(EntityID entity) override
	{
		data.Remove(entity);
	}

	bool Has(EntityID entity) const override
	{
		return data.Has(entity);
	}

	size_t Size() const override
	{
		return data.Size();
	}
};
class Scene
{
public:
	Scene();
	~Scene() noexcept;

private:
	EntityID nextEntityId = 0;
	std::vector<EntityID> reverseEntityMap; //Location in Alive[reverseEntityMap[EntityID]] 
	std::vector<EntityID> aliveEntities;
	std::vector<EntityID> EntitiesFreeSlots;

	std::vector<std::unique_ptr<IComponentStorage>> storages;
public:
	EntityID CreateEntity();
	void DestroyEntity(EntityID _entity);

	template<class T>
	EntityID CreateEntity(T _toAdd);

public:
	template<class TValue>
	void RegisterComponentStorage();

	template<class TValue>
	DenseStorage<EntityID,TValue>& GetComponentStorage();

	template<typename... Ts, typename Fn>
	inline void ForEach(Fn&& fn);

	template<class TValue>
	TValue& GetEntityComponent(EntityID _id);

	std::vector<EntityID>& GetAliveEntities(){ return aliveEntities; };

#ifdef VRAKTAL_EDITOR
public :
	void CallOnCreatedCallBacks(std::pair<EntityID, size_t> _data);
	void CallOnDestroyedCallBacks(std::pair<EntityID, size_t> _data);

	template <typename T>
	struct EntityChangeCallBack
	{
		void* context = nullptr; // Objet si data
		void (*callback)(void*,T) = nullptr;
		void Execute(const T& value) const
		{
			if (callback)
				callback(context, value);
		}
	};

	template<typename T, void(T::* Method)(std::pair<EntityID, size_t>)>
	void AddOnEntityCreatedCallBack(T* instance)
	{
		EntityChangeCallBack <std::pair<EntityID, size_t>> action;
		action.context = instance;
		action.callback = &MethodCaller<T, Method>;

		m_CreatedCallbacks.push_back(action);
	}

	template<typename T, void(T::* Method)(std::pair<EntityID, size_t>)>
	void AddOnEntityDestroyedCallBack(T* instance)
	{
		EntityChangeCallBack <std::pair<EntityID, size_t>> action;
		action.context = instance;
		action.callback = &MethodCaller<T, Method>;

		m_DestroyedCallbacks.push_back(action);
	}

private:
	std::vector<EntityChangeCallBack<std::pair<EntityID, size_t>>> m_CreatedCallbacks;
	std::vector<EntityChangeCallBack<std::pair<EntityID, size_t>>> m_DestroyedCallbacks;

	template<typename T, void(T::* Method)(std::pair<EntityID, size_t>)>
	static void MethodCaller(void* context,std::pair<EntityID, size_t> _pair)
	{
		T* obj = static_cast<T*>(context);
		(obj->*Method)(_pair);
	}

#endif // VRAKTAL_EDITOR


};

template<class T>
EntityID Scene::CreateEntity(T _toAdd)
{
	EntityID id = CreateEntity();
	GetComponentStorage<T>().Add(id , _toAdd);
	return id;
}

template<class TComponent>
DenseStorage<EntityID, TComponent>& Scene::GetComponentStorage()
{
    const auto id = ComponentTypeID<TComponent>();

    if (storages.size() <= id || !storages[id])
    {
        RegisterComponentStorage<TComponent>();
    }

    auto* storageData = static_cast<ComponentStorageData<TComponent>*>(storages[id].get());

    return storageData->data;
}

template<class T>
void Scene::RegisterComponentStorage()
{
	const auto id = ComponentTypeID<T>();

	if (storages.size() <= id)
	{
		storages.resize(id + 1);
	}

	if (!storages[id])
	{
		storages[id] = std::make_unique<ComponentStorageData<T>>();
	}
}

template<class TComponent>
inline TComponent& Scene::GetEntityComponent(EntityID _id)
{
	const auto id = ComponentTypeID<TComponent>();
	if (id >= storages.size() || !storages[id])
		throw std::runtime_error("ComponentStorage<T> not declared");

	return GetComponentStorage<TComponent>().Get(_id);
}

namespace ecs::detail
{
	template<class Tuple, class F, std::size_t...Is>
	inline void TupleForEachIndexedImpl(Tuple&& t, F&& f, std::index_sequence<Is...>)
	{
		(f(std::get<Is>(t), Is), ...);
	}

	template<class Tuple, class F>
	inline void TupleForEachIndexed(Tuple&& t, F&& f)
	{
		constexpr std::size_t N = std::tuple_size_v<std::remove_reference_t<Tuple>>;
		TupleForEachIndexedImpl(std::forward<Tuple>(t), std::forward<F>(f),
			std::make_index_sequence<N>{});
	}
}


template<typename... Ts, typename Fn>
inline void Scene::ForEach(Fn&& fn)
{
	static_assert(sizeof...(Ts) > 0, "ForEach needs at least one component type");

	auto storages = std::tuple{ &GetComponentStorage<Ts>()... };

	std::size_t driverIndex = 0;
	std::size_t minSize = std::numeric_limits<std::size_t>::max();

	ecs::detail::TupleForEachIndexed(storages, [&](auto* s, std::size_t i)
		{
			const std::size_t sz = s->Size();
			if (sz < minSize)
			{
				minSize = sz;
				driverIndex = i;
			}
		});

	const std::vector<EntityID>* driverEntities = nullptr;
	ecs::detail::TupleForEachIndexed(storages, [&](auto* s, std::size_t i)
		{
			if (i == driverIndex) driverEntities = &s->Entities();
		});

	if (!driverEntities || driverEntities->empty())
		return;

	for (EntityID e : *driverEntities)
	{
		const bool ok = std::apply([&](auto*... s)
			{
				return (s->Has(e) && ...);
			}, storages);

		if (!ok) continue;

		std::apply([&](auto*... s)
			{
				fn(e, s->Get(e)...);
			}, storages);
	}
}