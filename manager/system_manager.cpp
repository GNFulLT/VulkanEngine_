#include "system_manager.h"

#include "memory_manager.h"


void SystemManager::destroy()
{
	
	MemoryManager::get_singleton()->delete_custom_object(this);
	
}

bool SystemManager::init()
{
	return true;
}