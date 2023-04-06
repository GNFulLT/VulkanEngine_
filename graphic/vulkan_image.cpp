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

	if (!create_image_view(RenderDevice::get_singleton()->get_render_device().logicalDevice, m_image, IMAGE_FORMAT_TO_VK_FORMAT(image->format), VK_IMAGE_ASPECT_COLOR_BIT,
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

	if (!create_buffer(RenderDevice::get_singleton()->get_render_device().logicalDevice, RenderDevice::get_singleton()->get_render_device().physicalDev.physicalDev,
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

	auto command = RenderDevice::get_singleton()->begin_single_time_command();

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		(unsigned int)image->width,
		(unsigned int)image->height,
		1
	};

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = m_image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

	auto  sourceStage = VK_PIPELINE_STAGE_HOST_BIT;
	auto destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

	vkCmdPipelineBarrier(
		command,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);


	vkCmdCopyBufferToImage(
		command,
		stagingBuffer,
		m_image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

	vkCmdPipelineBarrier(
		command,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	//X TODO : NEED ASYNC WITH OPTIMAL RESORUCE LOADING QUEUE
	RenderDevice::get_singleton()->finish_exec_single_time_command(command);

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