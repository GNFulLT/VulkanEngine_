#ifndef FILE_NOT_FOUND_EX_H
#define FILE_NOT_FOUND_EX_H

#include <exception>
#include <string>

class FileNotFoundException : std::exception
{
public:
	explicit FileNotFoundException(const char* message)
		: m_msg	(message) {}
	explicit FileNotFoundException(const std::string& message)
		: m_msg(message) {}

	virtual const char* what() const noexcept {
		return m_msg.c_str();
	}

protected:
	/** Error message.
	 */
	std::string m_msg;
};

#endif // FILE_NOT_FOUND_EX_H
