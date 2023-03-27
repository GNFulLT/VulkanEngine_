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
	vkCmdBeginRenderPass(buff, get_main_renderpass_begin_inf(m_render_pass,m_frame_buffer), VK_SUBPASS_CONTENTS_INLINE);


	vkCmdEndRenderPass(buff);
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
				if (m_render_pass != nullptr)
				{
					vkDestroyRenderPass(m_dev, m_render_pass, nullptr);
					if (m_frame_buffer != nullptr)
					{
						vkDestroyFramebuffer(m_dev, m_frame_buffer, nullptr);
						if (m_image_sampler != nullptr)
						{
							vkDestroySampler(m_dev, m_image_sampler, nullptr);
						}
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

	// the renderpass will use this color attachment.
	VkAttachmentDescription color_attachment = {};
	//the attachment will have the format needed by the swapchain
	color_attachment.format = RenderDevice::get_singleton()->get_instance().format.format;
	//1 sample, we won't be doing MSAA
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	// Clear while loading
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// keep end state
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	//X TODO:  No stencil for now
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	// Used as frame buffer
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	// Texture will be used shader for read only to display in scene window
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


	VkAttachmentReference color_attachment_ref = {};
	//attachment number will index into the pAttachments array in the parent renderpass itself
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//we are going to create 1 subpass, which is the minimum you can do
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;



	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

	//connect the color attachment to the info
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment;
	//connect the subpass to the info
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;

	if (VK_SUCCESS != vkCreateRenderPass(m_dev, &render_pass_info, nullptr, &m_render_pass))
	{
		return false;
	}

	VkFramebufferCreateInfo fb_info = {};
	fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fb_info.pNext = nullptr;

	fb_info.renderPass = m_render_pass;
	fb_info.attachmentCount = 1;
	fb_info.width = m_size.x;
	fb_info.height = m_size.y;
	fb_info.layers = 1;


	fb_info.pAttachments = &m_image_view;
	if (vkCreateFramebuffer(m_dev, &fb_info, nullptr, &m_frame_buffer) != VK_SUCCESS)
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
				if (m_render_pass != nullptr)
				{
					vkDestroyRenderPass(m_dev, m_render_pass, nullptr);
					if (m_frame_buffer != nullptr)
					{
						vkDestroyFramebuffer(m_dev, m_frame_buffer, nullptr);
						if (m_image_sampler != nullptr)
						{
							vkDestroySampler(m_dev, m_image_sampler, nullptr);
						}
					}
				}
			}
		}

	}
}


void RenderScene::render_ex(VkQueue queue, VkCommandBuffer buff,VkFence fence)
{
	auto res = vkBeginCommandBuffer(buff, get_main_begin_inf());
	vkCmdBeginRenderPass(buff, get_main_renderpass_begin_inf(m_render_pass,m_frame_buffer), VK_SUBPASS_CONTENTS_INLINE);


	vkCmdEndRenderPass(buff);
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