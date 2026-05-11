#pragma once

#include <vector>
#include <unordered_map>
#include <cstdint>

template<typename TRessource>
struct RessourceStorage //Goal is AssetId -> Ressources depending on AssetID
{
	RessourceStorage(){};
	

	std::unordered_map<uint32_t , std::vector<T>>
};