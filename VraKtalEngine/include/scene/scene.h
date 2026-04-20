#pragma once
#include <iostream>
#include <tuple>
#include <utility>
#include <type_traits>


#include <utils/denseStorage.h>
#include "timeline/entityBase.h"

class Scene
{
private:
	std::vector<std::unique_ptr<BaseComponentStorage>> storages;
	static constexpr uint32_t INVALID = 0xFFFFFFFFu;
public:
	Scene();
	~Scene() noexcept;

private:
	EntityID nextEntityId = 0;
	std::vector<EntityID> reverseEntityMap; //Location in Alive[reverseEntityMap[EntityID]] 
	std::vector<EntityID> aliveEntities;
	std::vector<EntityID> EntitiesFreeSlots;

public:
	EntityID CreateEntity();
	void DestroyEntity(EntityID _entity);

	template<class T>
	EntityID CreateEntity(T _toAdd);

public:
	template<class T>
	void RegisterComponentStorage();

	template<class T>
	std::size_t ComponentTypeID();

	template<class T>
	ComponentStorage<T>& GetComponentStorage();

	template<typename... Ts, typename Fn>
	inline void ForEach(Fn&& fn);

private:
	inline std::size_t NextComponentTypeId()
	{
		static std::size_t next = 0;
		return next++;
	};

};

template<class T>
EntityID Scene::CreateEntity(T _toAdd)
{
	EntityID id = CreateEntity();
	GetComponentStorage<T>().Add(id , _toAdd);
	return id;
}

template<class T>
std::size_t Scene::ComponentTypeID()
{
	static std::size_t id = NextComponentTypeId();
	return id;
}

template<class T>
ComponentStorage<T>& Scene::GetComponentStorage()
{
	const auto id = ComponentTypeID<T>();
	if (id >= storages.size() || !storages[id])
		throw std::runtime_error("ComponentStorage<T> not declared");

	return *static_cast<ComponentStorage<T>*>(storages[id].get());
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
		storages[id] = std::make_unique<ComponentStorage<T>>();
	}
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