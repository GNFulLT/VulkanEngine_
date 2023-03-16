#ifndef SERIALIZABLE_STRUCT_H
#define SERIALIZABLE_STRUCT_H

#include <unordered_map>
#include "../string/string_types.h"

class ConfigurationServer;

template<typename Struct, typename T>
struct Property
{
    constexpr Property(T Struct::* aMember, const char* aName) : member{ aMember },name(aName) {}

    typedef T value_type;

    T Struct::* member;
    const char* name;
};


template<typename Struct, typename T>
constexpr Property<Struct,T> serializable_prop(T Struct::* member, const char* name) {
    return Property<Struct, T>{member, name};
}

template <typename T, T... S, typename F>
constexpr void for_sequence(std::integer_sequence<T, S...>, F&& f) {
    (static_cast<void>(f(std::integral_constant<T, S>{})), ...);
}


#define PROPERTY(CLASS, MEMBER) serializable_prop(&CLASS::MEMBER, #MEMBER)

#define SERIALIZABLE_STRUCT(...) \
private:                           \
    constexpr static auto properties = std::make_tuple(__VA_ARGS__); \
    friend ConfigurationServer; \
public:

#endif // SERIALIZABLE_STRUCT_H