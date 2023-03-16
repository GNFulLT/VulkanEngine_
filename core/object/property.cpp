#include "property.h"

#include "../../manager/memory_manager.h"

Property::~Property()
{
	if (m_defaultArgument == m_argument)
		MemoryManager::get_singleton()->delete_argument(&m_argument);
	else
	{
		MemoryManager::get_singleton()->delete_argument(&m_defaultArgument);
		MemoryManager::get_singleton()->delete_argument(&m_argument);
	}
}

bool Property::can_set_data() const noexcept
{		
	return (m_usage & PROPERTY_USAGE::PROPERTY_USAGE_READ_ONLY) == PROPERTY_USAGE::PROPERTY_USAGE_READ_ONLY;	
}
