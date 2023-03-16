#include "logger.h"
#include <spdlog/spdlog.h>
#include "../typedefs.h"

#undef ERROR


void Logger::select_log_level(LOG_LEVEL logLevel)
{
	switch (logLevel)
	{
	case LOG_LEVEL::INFO:
		spdlog::set_level(spdlog::level::info);
		break;
	case LOG_LEVEL::WARNING:
		spdlog::set_level(spdlog::level::warn);
		break;
	case LOG_LEVEL::CRITICAL:
		spdlog::set_level(spdlog::level::critical);
		break;
	case LOG_LEVEL::ERROR:
		spdlog::set_level(spdlog::level::err);
		break;
	case LOG_LEVEL::DEBUG:
		spdlog::set_level(spdlog::level::debug);
		break;
	case LOG_LEVEL::SHADER:
		break;
	}
}

void Logger::log(const char* msg, LOG_LEVEL level)
{
	switch (level)
	{
	case LOG_LEVEL::INFO:
		spdlog::log(spdlog::level::info, msg);
		break;
	case LOG_LEVEL::WARNING:
		spdlog::log(spdlog::level::warn, msg);
		break;
	case LOG_LEVEL::CRITICAL:
		spdlog::log(spdlog::level::critical, msg);
		break;
	case LOG_LEVEL::ERROR:
		spdlog::log(spdlog::level::err, msg);
		break;
	case LOG_LEVEL::DEBUG:
		spdlog::log(spdlog::level::debug, msg);
		break;
	case LOG_LEVEL::SHADER:
		break;
	}
}

Logger::LOG_LEVEL Logger::get_log_level()
{
	switch (spdlog::get_level())
	{
	case spdlog::level::info:
		return  LOG_LEVEL::INFO;
		break;
	case spdlog::level::warn:
		return LOG_LEVEL::WARNING;
		break;
	case spdlog::level::critical:
		return LOG_LEVEL::CRITICAL;
	case spdlog::level::err:
		return LOG_LEVEL::ERROR;
		break;
	case spdlog::level::debug:
		return LOG_LEVEL::DEBUG;
		break;
	case  spdlog::level::off:
		return Logger::OFF;
	default:
		return Logger::OFF;
	}
	return Logger::OFF;
}
bool Logger::should_log(LOG_LEVEL level)
{
	return true;
}
