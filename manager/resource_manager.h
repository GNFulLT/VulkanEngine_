#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "system_manager.h"

#include "../core/typedefs.h"
#include "../core/io/resource.h"
#include "../core/io/base_image_loader.h"
#include "../core/string/string_types.h"
#include <unordered_map>

class ResourceManager : public SystemManager
{
	SINGLETON(ResourceManager)
	OBJECT_DEF(ResourceManager,SystemManager)
public:
	ResourceManager();


	Resource* create_image_resource(const String& resourceKey,const String& resourcePath, Object* who = nullptr , RESOURCE_USAGE usage = RESOURCE_USAGE_NILL,const String& loaderName = "");
	
	//X Editor Usage added automatically
	Resource* create_editor_image_resource(const String& resourceKey, const String& resourcePath, Object* who = nullptr, RESOURCE_USAGE usage = RESOURCE_USAGE_NILL, const String& loaderName = "");
	
	void destroy_resource(Resource* res);
private:
	std::unordered_map<size_t, Resource*> m_resourceKeysMap;
	std::unordered_map<size_t, Resource*> m_resourcePathMap;
	std::unordered_map<size_t, ImageLoader*> m_imageLoaderMap;

	ImageLoader* m_defaultImageLoader;

	BaseImageLoader m_baseImageLoader;
};
#endif // RESOURCE_MANAGER_H