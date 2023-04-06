#ifndef FILE_TYPE_H
#define FILE_TYPE_H

#include "io_utils.h"
#include <boost/filesystem.hpp>
#include <boost/iterator.hpp>

enum FILE_TYPE
{
	FILE_TYPE_UNKNOWN = 0,
	FILE_TYPE_TXT,
	FILE_TYPE_FOLDER
};

_F_INLINE_ FILE_TYPE file_name_to_file_type(const boost::filesystem::path& path) noexcept
{
	if (!boost::filesystem::is_regular_file(path))
	{
		return FILE_TYPE_FOLDER;
	}
	if (path.extension() == ".txt")
	{
		return FILE_TYPE_TXT;
	}
	else
	{
		return FILE_TYPE_UNKNOWN;
	}
}

#endif // FILE_TYPE_H