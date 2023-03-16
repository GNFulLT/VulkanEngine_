#ifndef PROPERTY_H
#define PROPERTY_H

#include "argument.h"
#include "../string/string_types.h"
#include "object.h"

class MemoryManager;

enum PROPERTY_HINT
{
	PROPERTY_HINT_NILL, // No property hint given
	PROPERTY_HINT_RESOURCE_TYPE, // There is no owner for this types. It is binded to game directly
	PROPERTY_HINT_LOCALIZABLE_STRING // It is using as localizable string and connected with localization manager
};

enum PROPERTY_USAGE
{
	PROPERTY_USAGE_NILL = 0,
	PROPERTY_USAGE_STORAGE = 1 << 1, // Can be edit
	PROPERTY_USAGE_CONFIG = 1 << 2,	// It is being used for config in class
	PROPERTY_USAGE_READ_ONLY = 1 << 3 // Readonly	
};

class Property
{
public:

	_F_INLINE_ const char* get_name_c_str()
	{
		return m_name.c_str();
	}

	_F_INLINE_ void* get_raw_data()
	{
		return m_argument->get_raw();
	}

	template<typename T>
	_F_INLINE_ T* get()
	{
		auto pData = m_argument->get<T>();
		return pData;
	}

	_F_INLINE_ Type get_type()
	{
		return m_argument->get_type();
	}

	Property()
	{
		m_owner = nullptr;
		m_argument = nullptr;
		m_defaultArgument = nullptr;
		m_usage = PROPERTY_USAGE_NILL;
		m_hint = PROPERTY_HINT_NILL;
	}

	Property(Object* owner, Argument* arg, const char* name, PROPERTY_USAGE usage, PROPERTY_HINT hint)
		:
		m_owner(owner),
		m_argument(arg),
		m_usage(usage),
		m_hint(hint),
		m_defaultArgument(arg),
		m_name(name)
	{

	}

	virtual ~Property();
protected:
	
	template<typename T>
	_F_INLINE_ void change(const T& val)
	{
		m_argument->change(val);
	}

	virtual bool can_set_data() const noexcept;


	
	PROPERTY_HINT m_hint;
	PROPERTY_USAGE m_usage;
	Argument* m_argument;
	Argument* m_defaultArgument;
private:
	friend MemoryManager;
	StringName m_name; // Prop Name
	Object* m_owner; // Owner Ptr
};

#endif // PROPERTY_H