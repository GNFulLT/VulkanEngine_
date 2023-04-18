#include "render_scene.h"
#include "../../core/utils_vulkan.h"
#include "../../imgui/imgui_impl_vulkan.h"
#include "render_device.h"

RenderScene::RenderScene()
{
	m_dev = RenderDevice::get_singleton()->get_render_device().logicalDevice;
	m_physical_dev = RenderDevice::get_singleton()->get_render_device().physicalDev.physicalDev;
}



void RenderScene::on_resize(const GNF_UVec2& size, VulkanRenderpass* renderpass )
{
	destroy();
	init(size, renderpass);
}

bool RenderScene::fill_cmd(VkCommandBuffer buff)
{
	assert(false);
	/*auto res = vkBeginCommandBuffer(buff, get_main_begin_inf());
	m_renderPass->begin(buff);

	m_renderPass->end(buff);
	res = vkEndCommandBuffer(buff);*/

	return true;
}

void RenderScene::delete_buff()
{
	if (m_image_cpu_mem != nullptr)
	{
		vkDestroyBuffer(RenderDevice::get_singleton()->get_render_device().logicalDevice, m_image_cpu_buff, nullptr);
		vkFreeMemory(RenderDevice::get_singleton()->get_render_device().logicalDevice, m_image_cpu_mem, nullptr);
	}
}

uint8_t* RenderScene::get_image_bytes()
{
	auto cmd = RenderDevice::get_singleton()->begin_single_time_command();

	VkBuffer buffer;
	VkDeviceMemory mem;
	create_buffer(RenderDevice::get_singleton()->get_render_device().logicalDevice, RenderDevice::get_singleton()->get_render_device().physicalDev.physicalDev,
		m_size.x * m_size.y * 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, mem);


	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
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
		cmd,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);


	VkBufferImageCopy buffImgCopy = {};
	buffImgCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	buffImgCopy.imageSubresource.layerCount = 1;
	buffImgCopy.imageExtent.depth = 1;
	buffImgCopy.imageExtent.height = m_size.y;
	buffImgCopy.imageExtent.width = m_size.x;
	vkCmdCopyImageToBuffer(cmd, m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer, 1, &buffImgCopy);


	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL ;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = 0;
	sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	destinationStage = VK_PIPELINE_STAGE_HOST_BIT;

	vkCmdPipelineBarrier(
		cmd,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);



	RenderDevice::get_singleton()->finish_exec_single_time_command(cmd);

	uint8_t* imageData;
	vkMapMemory(RenderDevice::get_singleton()->get_render_device().logicalDevice,mem,0,VK_WHOLE_SIZE,0,(void**) & imageData);

	m_image_cpu_buff = buffer;
	m_image_cpu_mem = mem;

	return imageData;
}

RenderScene::~RenderScene()
{
	if (m_image != nullptr)
	{
		vkDestroyImage(m_dev, m_image, nullptr);
		if (m_image_memory != nullptr)
		{
			vkFreeMemory(m_dev, m_image_memory, nullptr);
			if (m_image_view != nullptr)
			{
				vkDestroyImageView(m_dev, m_image_view, nullptr);
				if (m_offScreenRenderingEnabled && !m_renderPass->is_failed())
				{
					m_renderPass->destroy(m_dev);
					if (m_image_sampler != nullptr)
					{
						vkDestroySampler(m_dev, m_image_sampler, nullptr);
					}

				}
			}
		}

	}
}

bool RenderScene::init(const GNF_UVec2& initial_size, VulkanRenderpass* renderpass )
{
	if (m_dev == nullptr)
		return false;

	m_size = initial_size;

	// If therre is no given renderpass
	// Do offrender
	if (renderpass == nullptr)
	{
		m_renderPass = MemoryManager::get_singleton()->fnew<VulkanRenderpass>();
		if (!create_image(m_dev, m_physical_dev, initial_size.x, initial_size.y, RenderDevice::get_singleton()->get_instance().format.format,
			VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_image, m_image_memory))
		{
			return false;
		}


		if (!create_image_view(m_dev, m_image, RenderDevice::get_singleton()->get_instance().format.format, VK_IMAGE_ASPECT_COLOR_BIT, &m_image_view, VK_IMAGE_VIEW_TYPE_2D, 1, 1))
		{
			return false;
		}
		std::vector<VkClearValue> clearValues;
		clearValues.push_back({ {0.f,1.f,0.f,1.f} });

		m_renderPass->create(m_dev, m_image_view, m_size.x, m_size.y, clearValues, RenderDevice::get_singleton()->get_instance().format.format,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		if (m_renderPass->is_failed())
		{
			return false;
		}

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;


		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(m_physical_dev, &properties);

		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;
		if (vkCreateSampler(m_dev, &samplerInfo, nullptr, &m_image_sampler) != VK_SUCCESS) {
			return false;
		}


		m_descriptor_set = ImGui_ImplVulkan_AddTexture(m_image_sampler, m_image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
	else
	{
		//X If there is given renderpass. Just use it

		m_renderPass = renderpass;
	}
			
	return true;

}

void RenderScene::destroy()
{
	if (m_image != nullptr)
	{
		vkDestroyImage(m_dev, m_image, nullptr);
		if (m_image_memory != nullptr)
		{
			vkFreeMemory(m_dev, m_image_memory, nullptr);
			if (m_image_view != nullptr)
			{
				vkDestroyImageView(m_dev, m_image_view, nullptr);
				if (m_offScreenRenderingEnabled && !m_renderPass->is_failed())
				{
					m_renderPass->destroy(m_dev);
					if (m_image_sampler != nullptr)
					{
						vkDestroySampler(m_dev, m_image_sampler, nullptr);
					}
					
				}
			}
		}

	}
}


void RenderScene::render_ex(VkQueue queue, VkCommandBuffer buff,VkFence fence)
{
	auto res = vkBeginCommandBuffer(buff, get_main_begin_inf());
	//vkCmdBeginRenderPass(buff, get_main_renderpass_begin_inf(m_render_pass,m_frame_buffer), VK_SUBPASS_CONTENTS_INLINE);
	m_renderPass->begin(buff);

	m_renderPass->end(buff);
	//vkCmdEndRenderPass(buff);
	res = vkEndCommandBuffer(buff);

	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo
	{
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,
		0,
		nullptr,
		&waitStage,
		0,
		nullptr,
		0,
		nullptr
	};

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &buff;


	res = vkQueueSubmit(queue, 1, &submitInfo,fence);

}