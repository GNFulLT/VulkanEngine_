#include "resource_manager.h"

#include "../core/string/string_utils.h"
#include "../graphic/vulkan_image.h"
#include "memory_manager.h"
ResourceManager::ResourceManager()
{
	m_imageLoaderMap.emplace(hash_string("DEFAULT"),&m_baseImageLoader); 
	m_defaultImageLoader = &m_baseImageLoader;

}

Resource* ResourceManager::create_image_resource(const String& resourceKey, const String& resourcePath, Object* who,const String& loaderName)
{
	//! CHECK IF GIVEN PATH RESOURCE IS LOADED BEFORE
	
	auto pathHash = hash_string(resourcePath);
	auto keyHash = hash_string(resourceKey);
	if (auto res = m_resourcePathMap.find(pathHash);res != m_resourcePathMap.end())
	{
		if (m_resourceKeysMap.find(keyHash) != m_resourceKeysMap.end())
		{
			return nullptr;
		}
		//m_resourceKeysMap.emplace(keyHash, res->second);
		return res->second;
	}
	
	//! Select Image Loader If any custom selected
	ImageLoader* imageLoader = m_defaultImageLoader;

	if (loaderName != "")
	{
		if (auto loader = m_imageLoaderMap.find(hash_string(loaderName)); loader != m_imageLoaderMap.end())
		{
			imageLoader = loader->second;
		}
	}	
	VulkanImage* vimg = MemoryManager::get_singleton()->new_object<VulkanImage>(resourceKey, imageLoader);
	//X TODO : LOG
	if (vimg->is_failed())
	{
		MemoryManager::get_singleton()->destroy_object(vimg);
		return nullptr;
	}

	vimg->register_owner(who);
	vimg->set_name(resourceKey.c_str());
	vimg->m_path = resourcePath;

	m_resourceKeysMap.emplace(keyHash,vimg);
	m_resourcePathMap.emplace(pathHash,vimg);

	return vimg;
}


void ResourceManager::destroy_resource(Resource* res)
{
	auto pathHash = hash_string(String(res->get_path()));
	auto keyHash = hash_string(String(res->get_object_name()));
	assert(m_resourceKeysMap.find(keyHash) != m_resourceKeysMap.end());
	assert(m_resourcePathMap.find(pathHash) != m_resourcePathMap.end());

	m_resourceKeysMap.erase(keyHash);
	m_resourcePathMap.erase(pathHash);


	MemoryManager::get_singleton()->destroy_object(res);
}