#ifndef PLUGIN_H
#define PLUGIN_H

#include "../core/object/object.h"
#include "../core/typedefs.h"
#include "../core/string/string_types.h"

enum GNFPLUGIN_TYPE
{
	GNFPLUGIN_NATIVE = 0
};

class GNFPlugin : public Object
{
	OBJECT_DEF(GNFPlugin, Object)

public:
	GNFPlugin(const String& pluginName, uint_fast32_t pluginVersion)
	{}

	GNFPlugin(const GNFPlugin&) = delete;
	GNFPlugin& operator=(const GNFPlugin&) = delete;

	virtual ~GNFPlugin() = default;

	_INLINE_ uint_fast32_t get_plugin_version()
	{
		return m_version;
	}

	_INLINE_ const char* get_plugin_name()
	{
		return m_name.c_str();
	}

	virtual void init() = 0;

	virtual void destroy() = 0;

	virtual GNFPLUGIN_TYPE get_plugin_type() = 0;

private:
	String m_name;
	uint_fast32_t m_version;
};


#endif // PLUGIN_H
