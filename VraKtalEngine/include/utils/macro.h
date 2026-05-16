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

#define VRAKTAL_RESOURCE_TRAITS(ResourceName,Factory)\
    template<>\
    struct ResourceTraits<graphics::resources::ResourceName>\
    {\
        using AssetType = graphics::assets::ResourceName;\
        using FactoryType = factory::##Factory;\
    };

#define VRAKTAL_DECLARE_GETTER_REF_F(Type, Name, Member)    \
    Type& Get##Name() { return Member; }                    \
    const Type& Get##Name() const { return Member; }        

#define VRAKTAL_DECLARE_GETTER_PTR_F(Type, Name, Member)    \
    Type* Get##Name() { return Member; }                    \
    const Type* Get##Name() const { return Member; }

#define VRAKTAL_DECLARE_GETTER_REF(Type, Name)  \
    Type& Get##Name();                          \
    const Type& Get##Name() const; 

#define VRAKTAL_DEFINE_GETTER_REF(Type, Name, Member , Class)   \
    Type& Class::Get##Name() { return Member; }                 \
    const Type& Class::Get##Name() const { return Member; }     

#define VRAKTAL_DECLARE_GETTER_PTR(Type, Name)  \
    Type* Get##Name();                          \
    const Type* Get##Name() const;

#define VRAKTAL_DEFINE_GETTER_PTR(Type, Name, Member , Class)  \
    Type* Class::Get##Name() { return Member; }                \
    const Type* Class::Get##Name() const { return Member; }    

#define VRAKTAL_DECLARE_GETTER_CPY(Type, Name)  \
    Type Get##Name();                          \
    const Type Get##Name() const; 

#define VRAKTAL_DEFINE_GETTER_CPY(Type, Name, Member , Class)  \
    Type Class::Get##Name() { return Member; }                \
    const Type Class::Get##Name() const { return Member; }    