#include <scene/system/systemManager.h>
#include <scene/system/systemBase.h>
#include <typeindex>

SystemManager::~SystemManager()
{
	for (auto& var : m_systems) {
        delete var.second;
	}
}

void SystemManager::Update(Scene& _scene)
{
	for (auto& [ID, system] : m_systems)
	{
		if (system)
			system->Update(_scene);
		else
			std::cerr << "Warning: System with ID " << ID.name() << " is null!" << std::endl;
	}
}
