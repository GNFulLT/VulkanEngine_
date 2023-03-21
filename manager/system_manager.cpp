#include "system_manager.h"

#include "memory_manager.h"


void SystemManager::destroy()
{
	
	MemoryManager::get_singleton()->destroy_singleton_object(this);
	
}

bool SystemManager::init()
{
	return true;
}