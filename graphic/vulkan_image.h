#ifndef VULKAN_IMAGE_H
#define VULKAN_IMAGE_H

#include "../core/io/resource.h"
#include "../core/io/bitmap.h"
#include "../core/io/image_loader.h"
#include "../core/typedefs.h"

#include <vulkan/vulkan.h>

class VulkanImage : public Resource
{
	OBJECT_DEF(VulkanImage,Resource)
public:
	VulkanImage(ImageLoader* loader);
	
	~VulkanImage();

	_F_INLINE_ bool is_failed() const noexcept
	{
		return m_image == nullptr;
	}
	
	RESOURCE_TYPE get_resource_type() const noexcept override final;
protected:
	bool load_impl() override final;
private:
	VkImage m_image = nullptr;
	VkDeviceMemory m_imageMemory = nullptr;
	VkImageView m_imageView = nullptr;
	//X TODO : Need to be weak pointer
	ImageLoader* m_loader = nullptr;
};

#endif // VULKAN_IMAGE_H