#pragma once
#include <array>
#include <unordered_map>
#include <iostream>

#include <typeindex>
#include <typeinfo>
class Scene;

class SystemBase;
class RenderSystem;

class SystemManager
{
public:
	SystemManager() = default; // Default constructor
	~SystemManager();
	SystemManager(const SystemManager&) = delete; // Disable copy constructor
	SystemManager& operator=(const SystemManager&) = delete; // Disable assignment operator

public:
	void Update(Scene& _scene);
public:
	template<typename T>
	T* GetSystem() {
		static_assert(std::is_base_of<SystemBase, T>::value, "T must derive from SystemBase");

		return reinterpret_cast<T*>(m_systems[std::type_index(typeid(T))]);

		//J'aurais pus faire ca joliment mais bon...
	};

	template<typename T, typename... Args>
	void AddSystem(Args&&... args) {
		static_assert(std::is_base_of<SystemBase, T>::value, "T must derive from SystemBase");
		T* system = new T(std::forward<Args>(args)...);
		m_systems[std::type_index(typeid(T))] = reinterpret_cast<SystemBase*>(system);
	};


private:
	std::unordered_map<std::type_index, SystemBase*> m_systems;
};

