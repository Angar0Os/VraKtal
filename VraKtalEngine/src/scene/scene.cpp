#include <scene/scene.h>

Scene::Scene(){}

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
		reverseEntityMap.resize(nextEntityId + 1, INVALID);
		reverseEntityMap[nextEntityId] = (uint32_t)aliveEntities.size() - 1;
	}
	return aliveEntities.back();
}

void Scene::DestroyEntity(EntityID entityID)
{
	int index = reverseEntityMap[entityID];

	if (aliveEntities[index])
	{
		for (size_t i = 0; i < storages.size(); i++)
		{
			storages[i].get()->Remove(entityID);
		}
		// swap-remove dans alive
		uint32_t idx = reverseEntityMap[entityID];
		EntityID last = aliveEntities.back();

		aliveEntities[idx] = last;
		reverseEntityMap[last] = idx;

		aliveEntities.pop_back();
		reverseEntityMap[entityID] = INVALID;
		EntitiesFreeSlots.push_back(entityID);

	}
	else
	{
		std::cout << "Entity with ID: " << entityID << " does not exist." << std::endl;
	}
}