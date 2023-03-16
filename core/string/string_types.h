#ifndef STRING_TYPE
#define STRING_TYPE

#include <string>
#include "string_name.h"

typedef std::string String;
typedef std::u16string String16;
typedef std::u32string String32;

namespace std
{
	using u16stringstream = basic_stringstream<char16_t, char_traits<char16_t>, allocator<char16_t>>;
	using u32stringstream = basic_stringstream<char32_t, char_traits<char32_t>, allocator<char32_t>>;
}


typedef std::stringstream StringBuilder;
typedef std::u16stringstream StringBuilder16;
typedef std::u32stringstream StringBuilder32;

#endif // STRING_TYPE