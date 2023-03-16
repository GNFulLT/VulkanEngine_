#include "logger_manager.h"
#include <boost/format.hpp>

#include "memory_manager.h"
void LoggerManager::log_cout(const Object * who, const String & msg, Logger::LOG_LEVEL level) const
{
	m_coutLogger->log(boost::str(boost::format("%1% [Logged by %2%]") % msg % who->get_class_name()).c_str(), level);
}

void LoggerManager::log_cout(const Object* who, const char* msg, Logger::LOG_LEVEL level) const
{
	m_coutLogger->log(boost::str(boost::format("%1% [Logged by %2%]") % msg % who->get_class_name()).c_str(), level);
}


LoggerManager::LoggerManager()
{
	m_coutLogger.reset(new Logger());
}
