#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

template<typename TKey, typename TValue>
struct DenseStorage
{
public:
    using ID = uint32_t;
    static constexpr ID INVALID_ID = 0xFFFFFFFFu;

    TValue& Add(const TKey& key, TValue value)
    {
        auto it = m_keyToDense.find(key);
        if (it != m_keyToDense.end())
        {
            ID index = it->second;
            m_denseValues[index] = std::move(value);
            return m_denseValues[index];
        }

        ID denseIndex = static_cast<ID>(m_denseKeys.size());
        m_keyToDense.emplace(key, denseIndex);
        m_denseKeys.push_back(key);
        m_denseValues.emplace_back(std::move(value));
        return m_denseValues.back();
    }

    void Remove(const TKey& key)
    {
        auto it = m_keyToDense.find(key);
        if (it == m_keyToDense.end())
            return;

        ID toDeleteIndex = it->second;
        ID lastIndex = static_cast<ID>(m_denseKeys.size() - 1);

        if (toDeleteIndex != lastIndex)
        {
            TKey lastKey = m_denseKeys[lastIndex];

            m_denseKeys[toDeleteIndex] = std::move(m_denseKeys[lastIndex]);
            m_denseValues[toDeleteIndex] = std::move(m_denseValues[lastIndex]);

            // Important: update the moved key, not the old dense index.
            m_keyToDense[lastKey] = toDeleteIndex;
        }

        m_denseKeys.pop_back();
        m_denseValues.pop_back();
        m_keyToDense.erase(it);
    }

    bool Has(const TKey& key) const
    {
        return m_keyToDense.find(key) != m_keyToDense.end();
    }

    TValue& Get(const TKey& key)
    {
        auto it = m_keyToDense.find(key);

        if (it == m_keyToDense.end())
            throw std::runtime_error("DenseStorage::Get(): key not found");

        return m_denseValues[it->second];
    }

    const TValue& Get(const TKey& key) const
    {
        auto it = m_keyToDense.find(key);

        if (it == m_keyToDense.end())
            throw std::runtime_error("DenseStorage::Get() const: key not found");

        return m_denseValues[it->second];
    }

    ID GetDenseIndex(const TKey& key) const
    {
        auto it = m_keyToDense.find(key);

        if (it == m_keyToDense.end())
            return INVALID_ID;

        return it->second;
    }

    TValue& GetByDenseIndex(ID index)
    {
        assert(index < m_denseValues.size());
        return m_denseValues[index];
    }

    const TValue& GetByDenseIndex(ID index) const
    {
        assert(index < m_denseValues.size());
        return m_denseValues[index];
    }

    const TKey& GetKeyByDenseIndex(ID index) const
    {
        assert(index < m_denseKeys.size());
        return m_denseKeys[index];
    }

    size_t Size() const
    {
        return m_denseKeys.size();
    }

    bool Empty() const
    {
        return m_denseKeys.empty();
    }

    void Clear()
    {
        m_denseKeys.clear();
        m_denseValues.clear();
        m_keyToDense.clear();
    }

    const std::vector<TKey>& Keys() const { return m_denseKeys; }

    std::vector<TValue>& Values() { return m_denseValues; }
    const std::vector<TValue>& Values() const { return m_denseValues; }

private:
    std::vector<TKey> m_denseKeys;
    std::vector<TValue> m_denseValues;
    std::unordered_map<TKey, ID> m_keyToDense;
};
