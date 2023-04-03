#ifndef OBJECT_H
#define OBJECT_H

#include "object_id.h"
#include "../string/string_types.h"
#include "../typedefs.h"
#include <unordered_map>
#include <concepts>



class Property;
class MemoryManager;
class ObjectDB;

#define OBJECT_NAME_MAX_LEN 32

#define OBJECT_DEF(m_class,m_inherited)  \
public : \
virtual const String& get_class_name() const noexcept override { static String g_className = _STR(m_class); return g_className;}		 \
virtual const String& get_inherited_class_name() const noexcept override {static String g_inheritedClassName = _STR(m_inherited); return g_inheritedClassName;}	 \
private:   

class Object
{
public:
	virtual ~Object() = default;

	virtual const String& get_class_name() const noexcept = 0;
	virtual const String& get_inherited_class_name() const noexcept = 0;

	virtual const char* get_object_name() const noexcept
	{
		return m_object_name.c_str();
	}


	const std::unordered_map<String, Property*>* get_property_map() const noexcept
	{
		return &m_property_map;
	}
	
	const Property* try_to_find_property(String name) const noexcept
	{
		return m_property_map.find(name).operator->()->second;
	}

	_INLINE_ const char* get_memory_name() const noexcept
	{
		return m_memory_align_name.c_str();

	}

	_INLINE_ ObjectID get_object_id() const noexcept
	{
		return m_instance_id;
	}

	_INLINE_ bool operator==(const Object& pObj) const { return m_instance_id == pObj.m_instance_id; }
	_INLINE_ bool operator!=(const Object& pObj) const { return m_instance_id != pObj.m_instance_id; }
	_INLINE_ bool operator<(const Object& pObj) const { return m_instance_id < pObj.m_instance_id; }
protected:
	_INLINE_ bool register_property(String name, Property* p)
	{
		if (m_property_map.find(name) != m_property_map.end())
			return false;

		m_property_map.emplace(name, p);
		return true;
	}

private:
	friend class MemoryManager;
	friend class ObjectDB;
	_INLINE_ void set_name(const char* name)
	{
		m_object_name.set_str(name);
	}
	_INLINE_ void set_memory_name(const char* name)
	{
		m_memory_align_name.set_str(name);
	}

	//X TODO : NEED TO BE CONVERTED WEAK
	std::unordered_map<String, Property*> m_property_map;
	StringName m_object_name;
	StringName m_memory_align_name;
	ObjectID m_instance_id;
};

template<typename T>
concept object_type = std::convertible_to<T*, Object*>;

#endif