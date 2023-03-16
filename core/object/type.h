#ifndef TYPE_H
#define TYPE_H

#include "../serialize/serializable_enum.h"
#include "object.h"

BETTER_ENUM(Type,uint8_t,NILL,

	// Atomics
	BOOL,
	INT,
	FLOAT,
	STRING,
    ENUM,
	// Core Types
	OBJECT
)


class TypeUtil
{
public:
    template<typename T>
    static constexpr Type from_type() {
        if constexpr (std::is_same_v<T, bool>) {
            return Type::BOOL;
        }
        else if constexpr (std::is_integral_v<T>) {
            return Type::INT;
        }
        else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
            return Type::FLOAT;
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            return Type::STRING;
        }
        else if constexpr (std::is_base_of_v<Object, T>) {
            return Type::OBJECT;
        }
        else {
            return Type::NILL;
        }
    }
};


#endif // TYPE_H