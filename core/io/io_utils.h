#ifndef IO_UTILS_H
#define IO_UTILS_H

#include "../string/string_utils.h"
#include "../typedefs.h"
#include <boost/filesystem.hpp>

_INLINE_ bool is_path_exist(const String& path)
{
	return boost::filesystem::exists(path);
}

_INLINE_ bool is_path_exist(const String16& path)
{
	// Convert u16string to wstring
	std::wstring wpath = boost::locale::conv::utf_to_utf<wchar_t>(path);

	boost::filesystem::path fs_path(wpath);
	return boost::filesystem::exists(fs_path);
}
_INLINE_ bool is_path_exist(const std::wstring& path)
{
	boost::filesystem::path fs_path(path);
	return boost::filesystem::exists(fs_path);
}

_INLINE_ void create_path(const String16& path)
{
	auto wpath = string16_to_wstr(path);
	boost::filesystem::create_directory(wpath);
}
_INLINE_ void create_path(const String& path)
{
	boost::filesystem::create_directory(path);
}
_INLINE_ void create_path(const std::wstring& path)
{
	boost::filesystem::create_directory(path);
}

_INLINE_ bool is_file_path(const String& path)
{
	boost::filesystem::path fpath(path);
	return fpath.has_filename();
}

// Be sure of path existence and given path should be a filepath
_INLINE_ String get_file_extension(const String& path)
{
	boost::filesystem::path fpath(path);
	
	assert(fpath.has_filename());
	
	if (!fpath.has_extension())
		return "";
	return fpath.extension().string();
}

_INLINE_ bool is_file(const String& path)
{
	boost::filesystem::path fpath(path);

	return fpath.has_filename();
}
#endif
