#ifndef LOGGER_H
#define LOGGER_H

#include "../object/object.h"

#undef ERROR

class Logger
{
public:
	enum LOG_LEVEL
	{
		OFF,
		DEBUG,
		INFO,
		WARNING,
		ERROR,
		CRITICAL,
		SHADER
	};

	virtual ~Logger() = default;
	virtual void select_log_level(LOG_LEVEL logLevel);
	virtual void log(const char* msg, LOG_LEVEL level);
	virtual LOG_LEVEL get_log_level();
private:
	virtual bool should_log(LOG_LEVEL level);

};

#endif // LOGGER_H