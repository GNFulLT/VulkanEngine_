#include "vulkan_image.h"

#include "../core/utils_vulkan.h"
#include "../manager/rendering/render_device.h"
VulkanImage::VulkanImage(ImageLoader* loader)
{
	m_loader = loader;
}

RESOURCE_TYPE VulkanImage::get_resource_type() const noexcept
{
	return RESOURCE_TYPE_TEXTURE;
}
bool VulkanImage::load_impl()
{
	auto image = m_loader->load(get_path());

	if (image == nullptr)
		return false;

	if (!create_image(RenderDevice::get_singleton()->get_render_device().logicalDevice, RenderDevice::get_singleton()->get_render_device().physicalDev.physicalDev,
		image->width, image->height, IMAGE_FORMAT_TO_VK_FORMAT(image->format), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_image, m_imageMemory))
	{
		m_image = nullptr;
		return false;
	}

	if (create_image_view(RenderDevice::get_singleton()->get_render_device().logicalDevice, m_image, IMAGE_FORMAT_TO_VK_FORMAT(image->format), VK_IMAGE_ASPECT_COLOR_BIT,
		&m_imageView, VK_IMAGE_VIEW_TYPE_2D, 1, 1))
	{
		vkDestroyImage(RenderDevice::get_singleton()->get_render_device().logicalDevice, m_image, nullptr);
		vkFreeMemory(RenderDevice::get_singleton()->get_render_device().logicalDevice, m_imageMemory, nullptr);
		m_image = nullptr;
		return false;
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	//X TODO: The sum of the components of each image_format type is 32
	VkDeviceSize imageSize = image->width * image->width * 4;

	if (create_buffer(RenderDevice::get_singleton()->get_render_device().logicalDevice, RenderDevice::get_singleton()->get_render_device().physicalDev.physicalDev,
		imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory))
	{
		vkDestroyImage(RenderDevice::get_singleton()->get_render_device().logicalDevice, m_image, nullptr);
		vkFreeMemory(RenderDevice::get_singleton()->get_render_device().logicalDevice, m_imageMemory, nullptr);
		vkDestroyImageView(RenderDevice::get_singleton()->get_render_device().logicalDevice, m_imageView, nullptr);
		m_image = nullptr;
		return false;
	}


	//X Copy data to Graphic Card
	void* data;
	vkMapMemory(RenderDevice::get_singleton()->get_render_device().logicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, image->pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(RenderDevice::get_singleton()->get_render_device().logicalDevice, stagingBufferMemory);



	vkDestroyBuffer(RenderDevice::get_singleton()->get_render_device().logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(RenderDevice::get_singleton()->get_render_device().logicalDevice, stagingBufferMemory, nullptr);

	m_loader->unload(image);
}

VulkanImage::~VulkanImage()
{
	if (m_image != nullptr)
	{
		vkDestroyImage(RenderDevice::get_singleton()->get_render_device().logicalDevice, m_image, nullptr);
		vkFreeMemory(RenderDevice::get_singleton()->get_render_device().logicalDevice, m_imageMemory, nullptr);
		vkDestroyImageView(RenderDevice::get_singleton()->get_render_device().logicalDevice, m_imageView, nullptr);
	}
}