#pragma once

#define VRAKTAL_FORWARD_DECLARE_ASSET_RESOURCE_FACTORY(ResourceName, FactoryName) \
    namespace factory                                      \
    {                                                      \
        struct FactoryName;                                \
    }                                                      \
    namespace graphics                                     \
    {                                                      \
        namespace assets                                   \
        {                                                  \
            struct ResourceName;                           \
        }                                                  \
        namespace resources                                \
        {                                                  \
            struct ResourceName;                           \
        }                                                  \
    }

#define VRAKTAL_RESOURCE_TRAITS(ResourceName)                         \
    template<>                                                        \
    struct ResourceTraits<graphics::resources::ResourceName>          \
    {                                                                 \
        using AssetType = graphics::assets::ResourceName;             \
        using FactoryType = factory::ResourceName##Factory;           \
    };