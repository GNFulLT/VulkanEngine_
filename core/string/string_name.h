#ifndef STRING_NAME_H
#define STRING_NAME_H

#define STRING_NAME_MAX_LEN 255

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif 

#include "../typedefs.h"
class StringName
{

public:
	StringName()
	{
		m_len = 0;
		for (int i = 0; i < STRING_NAME_MAX_LEN;i++)
		{
			m_name[i] = 0;
		}
	}

	StringName(const char* name) : StringName()
	{	
		m_len =	(uint16_t) (strlen(name) > STRING_NAME_MAX_LEN ? STRING_NAME_MAX_LEN : strlen(name));
		strncpy(m_name, name, m_len);
	}

	_F_INLINE_ void set_str(const char* name)
	{
		m_len = (uint16_t)(strlen(name) > STRING_NAME_MAX_LEN ? STRING_NAME_MAX_LEN : strlen(name));
		strncpy(m_name, name, m_len);
	}

	_F_INLINE_ const char* c_str() const noexcept
	{
		return m_name;
	}

	_F_INLINE_ uint16_t get_max_len() const noexcept
	{
		return STRING_NAME_MAX_LEN;
	}

private:
	uint16_t m_len;
	char m_name[STRING_NAME_MAX_LEN];
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif 


#endif // STRING_NAME_H