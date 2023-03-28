#include "render_scene.h"
#include "../../core/utils_vulkan.h"
#include "../../imgui/imgui_impl_vulkan.h"
#include "render_device.h"

RenderScene::RenderScene()
{
	m_dev = RenderDevice::get_singleton()->get_render_device().logicalDevice;
	m_physical_dev = RenderDevice::get_singleton()->get_render_device().physicalDev.physicalDev;
}



void RenderScene::on_resize(const GNF_UVec2& size)
{
	destroy();
	init(size);
}

bool RenderScene::fill_cmd(VkCommandBuffer buff)
{
	auto res = vkBeginCommandBuffer(buff, get_main_begin_inf());
	m_renderPass.begin(buff);

	m_renderPass.end(buff);
	res = vkEndCommandBuffer(buff);

	return true;
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
				if (!m_renderPass.is_failed())
				{
					m_renderPass.destroy(m_dev);
					if (m_image_sampler != nullptr)
					{
						vkDestroySampler(m_dev, m_image_sampler, nullptr);
					}

				}
			}
		}

	}
}

bool RenderScene::init(const GNF_UVec2& initial_size)
{
	if (m_dev == nullptr)
		return false;

	m_size = initial_size;

	if(!create_image(m_dev, m_physical_dev, initial_size.x, initial_size.y, RenderDevice::get_singleton()->get_instance().format.format,
	VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
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

	m_renderPass.create(m_dev, m_image_view, m_size.x, m_size.y, clearValues, RenderDevice::get_singleton()->get_instance().format.format,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	if (m_renderPass.is_failed())
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
				if (!m_renderPass.is_failed())
				{
					m_renderPass.destroy(m_dev);
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
	m_renderPass.begin(buff);

	m_renderPass.end(buff);
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