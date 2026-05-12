#pragma once
#include <cstddef>

inline std::size_t NextComponentTypeId()
{
	static std::size_t next = 0;
	return next++;
};

template<class T>
std::size_t ComponentTypeID()
{
	static std::size_t id = NextComponentTypeId();
	return id;
}