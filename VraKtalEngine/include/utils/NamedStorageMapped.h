#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <algorithm>

struct InterfaceStorage {
    virtual ~InterfaceStorage() = default;
};

template <typename T>
class NamedStorageMap
{
public:
    using ID = uint32_t;
    static constexpr ID INVALID_ID = 0xFFFFFFFFu;

#ifdef VRAKTAL_EDITOR
    ~NamedStorageMap<T>() {
        std::cout << "storage " << typeid(T).name() << " deleted" << std::endl;
    };
#endif // VRAKTAL_EDITOR


public:
    ID Add(const std::string& _name)
    {
        auto it = nameToId.find(_name);
        if (it != nameToId.end())
            return it->second;

        ID id = 0;

        if (!freeIDs.empty())
        {
            id = freeIDs.back();
            freeIDs.pop_back();

            names[id] = _name;
            values[id] = T{};
        }
        else
        {
            id = static_cast<ID>(names.size());
            names.push_back(_name);
            values.emplace_back();
        }

        nameToId.emplace(_name, id);
        return id;
    }

    ID Add(const std::string& _name , T& _value)
    {
        auto it = nameToId.find(_name);
        if (it != nameToId.end())
            return it->second;

        ID id = 0;

        if (!freeIDs.empty())
        {
            id = freeIDs.back();
            freeIDs.pop_back();

            names[id] = _name;
            values[id] = std::move(_value);
        }
        else
        {
            id = static_cast<ID>(names.size());
            names.push_back(_name);
            values.emplace_back(std::move(_value));
        }

        nameToId.emplace(_name, id);
        return id;
    }

    ID Remove(const std::string& _name)
    {
        auto it = nameToId.find(_name);
        if (it == nameToId.end())
            return INVALID_ID;

        ID id = it->second;
        nameToId.erase(it);

        names[id].clear();
        values[id] = T{};
        freeIDs.push_back(id);

        return id;
    }

    bool Contains(const std::string& _name) const
    {
        return nameToId.find(_name) != nameToId.end();
    }

    bool IsValidIndex(ID id) const
    {
        return id < names.size() && !names[id].empty();
    }

    ID Find(const std::string& _name) const
    {
        auto it = nameToId.find(_name);
        if (it == nameToId.end())
            return INVALID_ID;
        return it->second;
    }

    ID Find(const T& _value) const
    {
        for (ID id = 0; id < static_cast<ID>(values.size()); ++id)
        {
            if (IsValidIndex(id) && values[id] == _value)
                return id;
        }

        return INVALID_ID;
    }

    T& Get(ID _id)
    {
        if (!IsValidIndex(_id))
            throw std::runtime_error("Invalid NamedStorageMap id");
        return values[_id];
    }

    const std::string& GetName(ID _id) const
    {
        if (!IsValidIndex(_id))
            throw std::runtime_error("Invalid NamedStorageMap id");
        return names[_id];
    }


    std::vector<const T*> GetActiveValues() const
    {
        std::vector<const T*> activeValues;
        activeValues.reserve(nameToId.size());

        for (ID id = 0; id < static_cast<ID>(values.size()); ++id)
        {
            if (IsValidIndex(id))
                activeValues.push_back(&values[id]);
        }

        return activeValues;
    }

    std::vector<T>& GetAllValues()
    {
        return values;
    }
    std::vector<ID>& GetAllIDs()
    {
        return freeIDs;
    }
    std::vector<std::string>& GetAllNames()
    {
        return names;
    }
    std::unordered_map<std::string, ID>& GetNameIdMap()
    {
        return nameToId;
    }

    void ChangeName(ID _id, const std::string& _newName)
    {
        if (!IsValidIndex(_id))
            throw std::runtime_error("Invalid NamedStorageMap id");
        const std::string& oldName = names[_id];
        auto it = nameToId.find(oldName);
        if (it != nameToId.end())
            nameToId.erase(it);
        names[_id] = _newName;
        nameToId.emplace(_newName, _id);
    }

private:
    std::vector<std::string> names;
    std::unordered_map<std::string, ID> nameToId;
    std::vector<T> values;
    std::vector<ID> freeIDs;
};

template<typename T>
struct Storage : InterfaceStorage
{
    NamedStorageMap<T> data;
};