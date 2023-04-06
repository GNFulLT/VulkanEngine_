#ifndef EDITOR_IMAGE_H
#define EDITOR_IMAGE_H

#include "vulkan_image.h"

class EditorImage : public VulkanImage
{
	OBJECT_DEF(EditorImage,VulkanImage)
public:
	EditorImage(ImageLoader* loader) : VulkanImage(loader)
	{}

	virtual ~EditorImage();

	virtual bool load_impl() override;

	_F_INLINE_ VkDescriptorSet get_image() const noexcept
	{
		return m_descriptorSet;
	}

private:
	VkSampler m_imageSampler = nullptr;
	VkDescriptorSet m_descriptorSet = nullptr;
};



#endif // EDITOR_IMAGE_H