#ifndef OBJECT_DB_H
#define OBJECT_DB_H

#include "../typedefs.h"
#include "object.h"

#include <boost/uuid/uuid.hpp>          
#include <boost/uuid/uuid_generators.hpp> 
#include <boost/uuid/uuid_io.hpp>        
#include <boost/functional/hash.hpp>
#include <unordered_map>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>

class ObjectDB
{
public:
	template<object_type Obj>	
	static void register_object(Obj* obj)
	{
		boost::lock_guard<boost::mutex> guard(m_mtx);

		auto suggestedId = boost::hash_value(m_generator());
		obj->m_instance_id.m_id = suggestedId;
		m_objectMap.emplace((uint64_t)suggestedId, obj);
	}

	template<object_type Obj>
	static void unregister_object(Obj* obj)
	{
		boost::lock_guard<boost::mutex> guard(m_mtx);
		m_objectMap.erase((uint64_t)obj->m_instance_id);
	}

private:
	_INLINE_ static std::unordered_map<uint64_t, Object*> m_objectMap;
	_INLINE_ static boost::uuids::random_generator m_generator = boost::uuids::random_generator();
	_INLINE_ static boost::mutex m_mtx;
};


#endif // 