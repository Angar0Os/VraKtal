#include <glm/glm.hpp>
#include <unordered_map>

template <typename T>
struct InputAxisCallback
{
	void* context = nullptr; // Objet si data
    void (*callback)(void*, const T&) = nullptr;

    void Execute(const T& value) const
    {
        if (callback)
            callback(context, value);
    }
};