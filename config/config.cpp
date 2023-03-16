#include "config.h"

bool Config::is_serializable()
{
	return false;
}

bool Config::serialize(const String& key, const String& path)
{
	return false;
}

bool Config::serialize(const String& path)
{
	return false;
}
