#ifndef VULKAN_RENDERPASS_H
#define VULKAN_RENDERPASS_H

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>
#include <vector>

#include "../core/typedefs.h"
#include "../core/vec2.h"

struct VulkanRenderpass
{	
	VulkanRenderpass()
	{
		_info = {};
		_isFailed = true;
		_subpassContents = VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE;
	}
	void create(VkDevice dev,VkImageView imageView,uint32_t width,uint32_t height, const std::vector<VkClearValue>& clearValues,
		VkFormat format,VkImageLayout finalLayout,VkImageLayout attachmentReferenceLayout,VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE)
	{
		_info = {};

		_subpassContents = subpassContents;

		_clearValues = clearValues;
		VkAttachmentDescription color_attachment = {};
		//the attachment will have the format needed by the swapchain
		color_attachment.format = format;
		//1 sample, we won't be doing MSAA
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;

		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		//X TODO:  No stencil for now
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		// Used as frame buffer
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		color_attachment.finalLayout = finalLayout;

		VkRenderPassCreateInfo create_inf = {};
		create_inf.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		

		VkAttachmentReference color_attachment_ref = {};
		//attachment number will index into the pAttachments array in the parent renderpass itself
		color_attachment_ref.attachment = 0;
		color_attachment_ref.layout = attachmentReferenceLayout;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment_ref;

		//connect the color attachment to the info
		create_inf.attachmentCount = 1;
		create_inf.pAttachments = &color_attachment;
		//connect the subpass to the info
		create_inf.subpassCount = 1;
		create_inf.pSubpasses = &subpass;
		VkRenderPass handle;

		if (VK_SUCCESS != vkCreateRenderPass(dev, &create_inf, nullptr, &handle))
		{
			_isFailed = true;
		}
		else
		{
			_isFailed = false;
		}
		if (!_isFailed)
		{
			VkFramebufferCreateInfo fb_info = {};
			fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			fb_info.pNext = nullptr;

			fb_info.renderPass = handle;
			fb_info.attachmentCount = 1;
			fb_info.width = width;
			fb_info.height = height;
			fb_info.layers = 1;
			fb_info.pAttachments = &imageView;
			VkFramebuffer frameBuffer;
			if (vkCreateFramebuffer(dev, &fb_info, nullptr, &frameBuffer) != VK_SUCCESS)
			{
				_isFailed = true;
			}
			else
			{
				_isFailed = false;
				_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				_info.renderPass = handle;
				_info.framebuffer = frameBuffer;
				_info.renderArea.extent.width = width;
				_info.renderArea.extent.height = height;
				_info.clearValueCount = (uint32_t)_clearValues.size();
				_info.pClearValues = _clearValues.data();
			}
		}
		


		
	}

	_F_INLINE_ void destroy(VkDevice dev) _NO_EXCEPT_
	{
		if (_info.renderPass != nullptr)
			vkDestroyRenderPass(dev, _info.renderPass, nullptr);
		if(_info.framebuffer != nullptr)
			vkDestroyFramebuffer(dev, _info.framebuffer, nullptr);
		_isFailed = true;
	}

	_F_INLINE_ void begin(VkCommandBuffer cmd) _NO_EXCEPT_
	{
		vkCmdBeginRenderPass(cmd, &_info, _subpassContents);
	}

	_F_INLINE_ void end(VkCommandBuffer cmd) _NO_EXCEPT_
	{
		vkCmdEndRenderPass(cmd);
	}

	_F_INLINE_ void set_size(const GNF_UVec2* size) noexcept
	{
		_info.renderArea.extent.width = (uint32_t)size->x;
		_info.renderArea.extent.height = (uint32_t)size->y;
	}

	_F_INLINE_ bool is_failed() const noexcept
	{
		return _isFailed;
	}
	


private:
	VkRenderPassBeginInfo _info;
	VkSubpassContents _subpassContents;
	std::vector<VkClearValue> _clearValues;
	std::vector<VkPipelineStageFlags> _stageFlags;
	bool _isFailed = false;
};

#endif // VULKAN_RENDERPASS_H