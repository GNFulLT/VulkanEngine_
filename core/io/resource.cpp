#include "resource.h"

void Resource::load()
{
	m_loaded = load_impl();
}

void Resource::register_owner(Object* owner)
{
	//X TODO : Mutex needed
	if (m_ownerSet.find((size_t)owner->get_object_id()) != m_ownerSet.end())
		return;

	m_ownerSet.insert((size_t)owner->get_object_id());
	m_owners.push_back(owner);
}

void Resource::unregister_owner(Object* owner)
{
	//X TODO : Mutex needed
	if (m_ownerSet.find((size_t)owner->get_object_id()) == m_ownerSet.end())
		return;

	auto iter = m_owners.begin();
	while (iter != m_owners.end())
	{
		if ((*iter) == owner)
		{
			m_owners.erase(iter);
			break;
		}
		iter++;
	}

	m_ownerSet.erase((size_t)owner->get_object_id());
}
