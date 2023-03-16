#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H


#include "../core/object/object.h"


class SystemManager : public Object
{
	OBJECT_DEF(SystemManager, Object)
public:
	~SystemManager() = default;
public:
	virtual void destroy();
	virtual bool init();
};

#endif // SYSTEM_MANAGER_H