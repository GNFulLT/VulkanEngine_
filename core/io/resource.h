#ifndef RESOURCE_H
#define RESOURCE_H

#include <set>
#include <vector>
#include "../object/object.h"
#include "../string/string_types.h"

class ResourceManager;

enum RESOURCE_USAGE
{
	RESOURCE_USAGE_SYSTEM // System Resource
};

enum RESOURCE_TYPE
{
	RESOURCE_TYPE_TEXTURE
};

class Resource : public Object
{
	OBJECT_DEF(Resource,Object)
public:
	virtual ~Resource() = default;
	void register_owner(Object* owner);
	void unregister_owner(Object* owner);
	virtual RESOURCE_TYPE get_resource_type() const noexcept = 0;

	void load();

	_F_INLINE_ const String& get_path() const noexcept
	{
		return m_path;
	}
	
	_F_INLINE_ bool is_loaded() const noexcept
	{
		return m_loaded;
	}

protected:
	virtual bool load_impl() = 0;
private:
	friend class ResourceManager;

	std::vector<Object*> m_owners;
	std::set<size_t> m_ownerSet;
	String m_path;
	RESOURCE_USAGE m_usage;
	bool m_loaded = false;
};

#endif // RESOURCE_H