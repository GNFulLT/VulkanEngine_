#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <type_traits>

#include "../core/typedefs.h"
#include "../core/object/object.h"
#include "../core/string/string_utils.h"
#include "../core/object/property.h"
#include "../core/vec2.h"
#include <boost/pool/singleton_pool.hpp>
#include <mimalloc.h>
#include <unordered_map>
#include <new>
#include <concepts>
#include <set>
#include <iostream>
#include <cassert>

#include <boost/uuid/uuid.hpp>          
#include <boost/uuid/uuid_generators.hpp> 
#include <boost/uuid/uuid_io.hpp>        
#include <boost/functional/hash.hpp>

#include "../core/object/object_db.h"


template<typename T>
concept TrivialConstructable = not std::is_trivially_constructible<T>::value;

class MemoryManager
{
	SINGLETON(MemoryManager)
public:

	MemoryManager()
	{
		m_allocated_blocks.insert(this);
	}

	~MemoryManager()
	{
		int a = 5;
		std::cout << "Deleted";
		if (m_allocated_blocks.size() != 0)
		{
			std::cout << "There is a thing that is not destroyed";
		}
	}

	_F_INLINE_ size_t* malloc_pool_size_t()
	{
		return (size_t*)m_size_t_double_mem.malloc();
	}
	
	//X TODO : Create Unique Ptr And Shared Ptr Smart Pointers 
	//_F_INLINE_ Property* create_property(bool initialVal,const char* name,PROPERTY_USAGE usage = PROPERTY_USAGE_STORAGE,PROPERTY_HINT hint = PROPERTY_HINT_NILL)
	//{
	//	bool* val = (bool*)m_bool_mem.malloc();
	//	*val = initialVal;

	//	void* pvArg = m_arg_mem.malloc();

	//	Argument* pArg = new (pvArg) Argument(val);
	//	
	//	void* pvProperty = m_property_mem.malloc();
	//	Property* pProperty = new (pvProperty) Property(nullptr, pArg, name, usage, hint);

	//	return pProperty;
	//}

	template<object_type Obj, typename... Args>
	_IMP_RETURN_ _INLINE_ Obj* new_object(const String& name, Args... args)
	{
		void* ptr = mi_malloc(sizeof(Obj));
		
		m_allocated_blocks.insert(ptr);
		
		Obj* oPtr = new (ptr) Obj(args...);
		
		ObjectDB::register_object(oPtr);

		return oPtr;
	}

	template<typename Obj>
	_IMP_RETURN_ _INLINE_ Obj* fnew_c()
	{
		void* ptr = mi_malloc(sizeof(Obj));

		m_allocated_blocks.insert(ptr);

		return (Obj*)ptr;
	}
	
	template<typename Obj>
	_INLINE_ void fdel_c(Obj* ptr)
	{
		// X TODO : NEED HANDLE
		/*assert(m_allocated_blocks.find(ptr) != m_allocated_blocks.end());
		assert(m_object_name_map.find(ptr) != m_object_name_map.end());*/
		m_allocated_blocks.erase(ptr);

		mi_free(ptr);
	}

	template<typename Obj, typename... Args>
	_IMP_RETURN_ _INLINE_ Obj* fnew(Args... args)
	{
		void* ptr = mi_malloc(sizeof(Obj));

		m_allocated_blocks.insert(ptr);

		Obj* oPtr = new (ptr) Obj(args...);

		return oPtr;
	}
	
	template<typename Obj>
	_INLINE_ void fdel(Obj* ptr)
	{
		// X TODO : NEED HANDLE
		/*assert(m_allocated_blocks.find(ptr) != m_allocated_blocks.end());
		assert(m_object_name_map.find(ptr) != m_object_name_map.end());*/
		m_allocated_blocks.erase(ptr);

		ptr->~Obj();
		mi_free(ptr);

	}

	template<object_type Obj, typename... Args>
	_F_INLINE_ Obj* create_singleton_object(const String& class_name,Args... args)
	{
		Object* object = (Object*)new_object<Obj>(class_name, args...);
		
		return (Obj*)object;	
	}

	template<object_type Obj>
	_INLINE_ void destroy_object(Obj* ptr)
	{
		// X TODO : NEED HANDLE
		/*assert(m_allocated_blocks.find(ptr) != m_allocated_blocks.end());
		assert(m_object_name_map.find(ptr) != m_object_name_map.end());*/

		ObjectDB::unregister_object(ptr);
		m_allocated_blocks.erase(ptr);

		ptr->~Obj();
		mi_free(ptr);
		
	}

	template<object_type Obj>
	_F_INLINE_ void destroy_singleton_object(Obj* ptr)
	{
		destroy_object<Obj>(ptr);
		
	}
	/*template<object_type T,typename... Args>
	_F_INLINE_ T* new_custom_object(Args... args)
	{
		void* ptr = mi_malloc(sizeof(T));
		return new (ptr) T(args...);
	}

	*/

	template<typename T, typename std::enable_if<!std::is_enum<T>::value, int>::type = 0>
	_F_INLINE_ Argument* create_argument(T initialVal);

	template<typename T, typename std::enable_if<std::is_enum<T>::value, int>::type = 0>
	_INLINE_ Argument* create_argument(const T& initialVal)
	{
		size_t* val = (size_t*)m_size_t_double_mem.malloc();
		*val = initialVal;

		void* pvArg = m_arg_mem.malloc();

		return new (pvArg) Argument(val);
	}
	

	template<>
	_F_INLINE_ Argument* create_argument<bool>(bool initialVal)
	{
		bool* val = (bool*)m_bool_mem.malloc();
		*val = initialVal;

		void* pvArg = m_arg_mem.malloc();

		return new (pvArg) Argument(val);
	}

	template<>
	_F_INLINE_ Argument* create_argument<size_t>(size_t initialVal)
	{
		size_t* val = (size_t*)m_size_t_double_mem.malloc();
		*val = initialVal;

		void* pvArg = m_arg_mem.malloc();

		return new (pvArg) Argument(val);
	}

	template<>
	_F_INLINE_ Argument* create_argument<int>(int initialVal)
	{
		size_t* val = (size_t*)m_size_t_double_mem.malloc();
		*val = initialVal;

		void* pvArg = m_arg_mem.malloc();

		return new (pvArg) Argument(val);
	}

	template<>
	_F_INLINE_ Argument* create_argument<GNF_UVec2>(GNF_UVec2 initialVal)
	{
		int a = sizeof(GNF_UVec2);
		int b = sizeof(size_t);
		GNF_UVec2* val = (GNF_UVec2*)m_size_t_double_mem.malloc();
		*val = initialVal;

		void* pvArg = m_arg_mem.malloc();

		return new (pvArg) Argument(val);
	}

	_F_INLINE_ Property create_property(bool initialVal, const char* name, PROPERTY_USAGE usage = PROPERTY_USAGE_STORAGE, PROPERTY_HINT hint = PROPERTY_HINT_NILL)
	{
		bool* val = (bool*)m_bool_mem.malloc();
		*val = initialVal;

		void* pvArg = m_arg_mem.malloc();

		Argument* pArg = new (pvArg) Argument(val);

		return Property(nullptr,pArg,name,usage,hint);
	}

	_F_INLINE_ void delete_argument(Argument** arg)
	{
		switch ((*arg)->m_type)
		{
		case Type::BOOL:
			m_bool_mem.free((*arg)->m_value);
			break;
		case Type::FLOAT:
			m_size_t_double_mem.free((*arg)->m_value);
			break;
		case Type::INT:
			m_size_t_double_mem.free((*arg)->m_value);
			break;
		case Type::STRING:
			//X TODO : StringName
			break;
		default:
			//X TODO : Unknown
			break;
		}

		// Now delete argument
		m_arg_mem.free(*arg);
		
		*arg = nullptr;
	}

	_F_INLINE_ bool is_singleton(void* ptr)
	{
		assert(false);
		return (m_allocated_blocks.find(ptr) != m_allocated_blocks.end());
	}

	_F_INLINE_ bool is_valid(void* ptr)
	{
		return (m_allocated_blocks.find(ptr) != m_allocated_blocks.end());
	}
	
	_F_INLINE_ void destroy()
	{
		dealloc<MemoryManager>(this);
	}

	// Statics
public:
	_INLINE_ static MemoryManager* create_singleton()
	{
		singleton = alloc_trivial_ctor<MemoryManager>();
		return singleton;
	}

	// Private Methods
private:
	// These for arguments
	boost::pool<> m_bool_mem = boost::pool<>(sizeof(bool));
	boost::pool<> m_size_t_double_mem = boost::pool<>(sizeof(size_t));
	boost::pool<> m_arg_mem = boost::pool<>(sizeof(Argument));
	
	template<object_type T>
	_F_INLINE_ void delete_custom_object(T* ptr)
	{
		ptr->~T();
		mi_free(ptr);
	}

	_INLINE_ static boost::uuids::random_generator generator = boost::uuids::random_generator();
	
	template<TrivialConstructable T>
	_F_INLINE_ static T* alloc_trivial_ctor()
	{
		uint64_t* mem_loc =  (uint64_t*)mi_malloc(sizeof(T));
		if(mem_loc != nullptr)
			new (((T*)mem_loc)) T;
		return (T*)mem_loc;
	}	

	template<typename T>
	_F_INLINE_ void dealloc(T* p)
	{
		assert(m_allocated_blocks.find(p) != m_allocated_blocks.end());
		p->~T();
		mi_free(p);
		p = nullptr;
	}

	// Private Variables
private:
	std::set<void*> m_allocated_blocks;

};


#endif // MEMORY_MANAGER_H