#include <scene/scene.h>

#ifdef VRAKTAL_EDITOR
#include <string>
#endif

Scene::Scene(std::string _name, bool _createNameStorage)
	: m_name(_name)
	, m_createdWithNameStorage(_createNameStorage)
{
	if (m_createdWithNameStorage)
		RegisterComponentStorage<std::string>();
}
Scene::~Scene() noexcept
{
	std::cout << "Destroying World instance..." << std::endl;

	// Clean up all active entities from end of array
	for (EntityID entityID : aliveEntities)
	{
		std::cout << "Cleaning up entity with ID: " << entityID << std::endl;
	}
}

EntityID Scene::CreateEntity()
{
		
	if (!EntitiesFreeSlots.empty())
	{

		aliveEntities.push_back(EntitiesFreeSlots.back());
		reverseEntityMap[EntitiesFreeSlots.back()] = (uint32_t)aliveEntities.size() - 1;
		EntitiesFreeSlots.pop_back();
	}
	else
	{
		nextEntityId++;
		aliveEntities.push_back(nextEntityId);
		reverseEntityMap.resize(nextEntityId + 1, INVALID_ENTITY);
		reverseEntityMap[nextEntityId] = (uint32_t)aliveEntities.size() - 1;
	}

	if (m_createdWithNameStorage)
	{
		std::string entityName = "entity" + std::to_string(aliveEntities.back());
		GetComponentStorage<std::string>().Add(aliveEntities.back(), entityName);
	}
	
	CallOnCreatedCallBacks({ aliveEntities.back() , aliveEntities.size() });
	
	return aliveEntities.back();
}

void Scene::DestroyEntity(EntityID entityID)
{
	CallOnDestroyedCallBacks({ entityID , reverseEntityMap[entityID] });
	int index = reverseEntityMap[entityID];

	if (aliveEntities[index])
	{
		for (size_t i = 0; i < storages.size(); i++)
		{
			if (storages[i])
			{
				storages[i]->Remove(entityID);

			}
		}
		// swap-remove dans alive
		uint32_t idx = reverseEntityMap[entityID];
		EntityID last = aliveEntities.back();

		aliveEntities[idx] = last;
		reverseEntityMap[last] = idx;

		aliveEntities.pop_back();
		reverseEntityMap[entityID] = INVALID_ENTITY;
		EntitiesFreeSlots.push_back(entityID);

	}
	else
	{
		std::cout << "Entity with ID: " << entityID << " does not exist." << std::endl;
	}
}

void Scene::CallOnCreatedCallBacks(std::pair<EntityID, size_t> _data)
{
	for (size_t i = 0; i < m_CreatedCallbacks.size(); i++)
	{
		m_CreatedCallbacks[i].Execute(_data);
	}
}

void Scene::CallOnDestroyedCallBacks(std::pair<EntityID, size_t> _data)
{
	for (size_t i = 0; i < m_DestroyedCallbacks.size(); i++)
	{
		m_DestroyedCallbacks[i].Execute(_data);
	}
}
