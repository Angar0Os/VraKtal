#pragma once
#include <scene/timeline/entityBase.h>
#include <vector>
#include <iostream>

struct BaseComponentStorage {
    virtual size_t Size() const = 0;
    virtual void Remove(EntityID id) = 0;
};

template<typename T>
struct ComponentStorage : public BaseComponentStorage
{
public:
    // Ajoute ou remplace
    T& Add(EntityID id, const T& component)
    {
        EnsureSparseSize(id);

        if (Has(id)) {
            denseComponents[FreeIndex[id]] = component;
            return denseComponents[FreeIndex[id]];
        }

        FreeIndex[id] = (uint32_t)denseEntities.size();
        denseEntities.push_back(id);
        denseComponents.push_back(component);
        return denseComponents.back();
    }

    void Remove(EntityID id) override
    {
        if (!Has(id)) return;

        uint32_t idx = FreeIndex[id];
        uint32_t lastIdx = (uint32_t)denseEntities.size() - 1;
        EntityID lastEntity = denseEntities[lastIdx];

        // swap-remove entities
        denseEntities[idx] = lastEntity;
        denseComponents[idx] = std::move(denseComponents[lastIdx]);

        // update mapping for moved entity
        FreeIndex[lastEntity] = idx;

        // pop
        denseEntities.pop_back();
        denseComponents.pop_back();

        // mark removed
        FreeIndex[id] = INVALID;
    }

    bool Has(EntityID id) const
    {
        return id < FreeIndex.size() && FreeIndex[id] != INVALID;
    }

    T& Get(EntityID id)
    {
        if (!Has(id)) throw std::runtime_error("Get(): component not found for entity");
        return denseComponents[FreeIndex[id]];
    }

    size_t Size() const override { return denseEntities.size(); }

    // Pour itérer efficacement
    const std::vector<EntityID>& Entities() const { return denseEntities; }
    std::vector<T>& Components() { return denseComponents; }
    const std::vector<T>& Components() const { return denseComponents; }

private:
    void EnsureSparseSize(EntityID id)
    {
        if (FreeIndex.size() <= id)
            FreeIndex.resize(id + 1, INVALID);
    }

private:
    std::vector<EntityID> denseEntities;    // entités qui ont T
    std::vector<T>        denseComponents;  // composants alignés
    std::vector<uint32_t> FreeIndex;      // EntityID -> index dense, ou INVALID
    static constexpr uint32_t INVALID = 0xFFFFFFFFu;
};
