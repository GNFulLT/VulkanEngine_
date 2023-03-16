#ifndef ARGUMENT_H
#define ARGUMENT_H

#include <concepts>
#include <type_traits>
#include <cassert>

#include "type.h"
#include "../typedefs.h"

//template<typename T>
//concept argument = std::is_integral_v<T> || std::is_same_v<T, std::string> || std::is_convertible_v<T, Object>;

class MemoryManager;

class Argument
{
public:
	Argument(const Argument&) = delete;
	Argument& operator=(const Argument&) = delete;

	_F_INLINE_ void* get_raw()
	{
		return m_value;
	}

	template<typename T>
	_F_INLINE_ void set(const T& val) const noexcept
	{
		m_value = val;
	}

	template<typename T>
	_F_INLINE_ T* get() const noexcept
	{
		return (T*)m_value;
	}

	_F_INLINE_ Type get_type() const noexcept
	{
		return m_type;
	}
	template<typename T, typename std::enable_if<!std::is_enum<T>::value, int>::type = 0>
	_F_INLINE_ void change(const T& val)
	{
		assert(m_type == TypeUtil::from_type<T>());
		*(static_cast<T*>(m_value)) = val;
	}
	template<typename T, typename std::enable_if<std::is_enum<T>::value, int>::type = 0>
	_F_INLINE_ void change(const T& val)
	{
		*(static_cast<T*>(m_value)) = val;
	}
private:
	template<typename T, typename std::enable_if<!std::is_enum<T>::value, int>::type = 0>
	Argument(T* arg) : m_value(arg)
	{
		m_type = TypeUtil::from_type<T>();
	}

	template<typename T, typename std::enable_if<std::is_enum<T>::value, int>::type = 0>
	Argument(T* arg) : m_value(arg)
	{
		m_type = Type::ENUM;
	}

private:
	friend class MemoryManager;
	Type m_type = Type::NILL;
	void* m_value;
};

#endif // ARGUMENT_H