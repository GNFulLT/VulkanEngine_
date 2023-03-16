#include "string_utils.h"

#include <boost/functional/hash.hpp>


size_t hash_string(const String& str)
{
	return boost::hash_value(str);
}