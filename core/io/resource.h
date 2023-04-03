#ifndef RESOURCE_H
#define RESOURCE_H

#include <set>
#include <vector>
#include "../object/object.h"
#include "../string/string_types.h"

enum RESOURCE_USAGE
{
	RESOURCE_USAGE_SYSTEM // System Resource
};

class Resource : public Object
{
	OBJECT_DEF(Resource,Object)
public:
	void register_owner(Object* owner);
	void unregister_owner(Object* owner);

	_F_INLINE_ const String& get_path() const noexcept
	{
		return m_path;
	}

private:
	std::vector<Object*> m_owners;
	std::set<size_t> m_ownerSet;
	String m_path;
	RESOURCE_USAGE m_usage;

};

#endif // RESOURCE_H