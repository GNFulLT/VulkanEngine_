#ifndef LOGGER_MANAGER_H
#define LOGGER_MANAGER_H


#include "../core/object/object.h"
#include "../core/typedefs.h"
#include "../core/io/logger.h"
#include "../core/string/string_utils.h"
#include "system_manager.h"

#include <memory>
#include <boost/format.hpp>


class LoggerManager : public SystemManager 
{
	SINGLETON(LoggerManager)
	OBJECT_DEF(LoggerManager, SystemManager)
public:

	LoggerManager();

	void log_cout(const Object* who, const String& msg, Logger::LOG_LEVEL level) const;
	void log_cout(const Object* who, const char* msg, Logger::LOG_LEVEL level) const;
	_INLINE_ Logger::LOG_LEVEL get_log_level_cout()
	{
		return m_coutLogger->get_log_level();
	}

	_INLINE_ void set_log_level_cout(Logger::LOG_LEVEL level)
	{
		m_coutLogger->select_log_level(level);
	}
private:
	friend class CreationServer;
	friend class ConfigurationServer;
	friend class EventBusServer;


	_INLINE_ void log_cout(const String& msg, Logger::LOG_LEVEL level)
	{
		m_coutLogger->log(msg.c_str(), level);

	}
	_INLINE_ void log_cout(const char* msg, Logger::LOG_LEVEL level)
	{
		m_coutLogger->log(msg, level);

	}

private:
	std::unique_ptr<Logger> m_coutLogger;
};


#endif // LOGGER_MANAGER_H