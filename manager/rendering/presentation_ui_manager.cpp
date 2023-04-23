#include "presentation_ui_manager.h"

#include <cmath>
#include <algorithm>

#include "../glfw_window.h"
#include "../logger_manager.h"
#include <vulkan/vulkan.h>

#include <boost/algorithm/clamp.hpp>

#include "render_device.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include <boost/bind/bind.hpp>

PresentationUIManager::PresentationUIManager()
{
	m_surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
	m_surfaceFormat.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
	m_imageCount = 3;
}

bool PresentationUIManager::init()
{
	if (!create_surface())
		return false;

	m_windowResizeConnection = WindowManager::get_singleton()->get_size()->subscribe_changed_event(boost::bind(&PresentationUIManager::on_window_resize, this, boost::placeholders::_1));

	get_swap_chain_support_details(RenderDevice::get_singleton()->get_render_device().physicalDev.physicalDev, m_surface, m_swapChainDetails);

	if (m_swapChainDetails.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		m_surfaceExtent = m_swapChainDetails.capabilities.currentExtent;
	}
	else {
		int wndSizeX = WindowManager::get_singleton()->get_size_r()->x;
		int wndSizeY = WindowManager::get_singleton()->get_size_r()->y;
		VkExtent2D actualExtent = {
			static_cast<uint32_t>(wndSizeX),
			static_cast<uint32_t>(wndSizeY)
		};
		
		actualExtent.width = boost::algorithm::clamp(actualExtent.width, m_swapChainDetails.capabilities.minImageExtent.width, m_swapChainDetails.capabilities.maxImageExtent.width);
		actualExtent.height = boost::algorithm::clamp(actualExtent.height, m_swapChainDetails.capabilities.minImageExtent.height, m_swapChainDetails.capabilities.maxImageExtent.height);

		m_surfaceExtent = actualExtent;
	}

	VkSwapchainCreateInfoKHR swapchain_create_info = {
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		nullptr,
		0,
		m_surface,
		(uint32_t)m_imageCount,
		m_surfaceFormat.format,
		m_surfaceFormat.colorSpace,
		m_surfaceExtent,
		1,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT , // Image Usage
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		nullptr,
		m_swapChainDetails.capabilities.currentTransform,
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		m_presentMode,
		VK_TRUE,
		m_swapchain.swapchain
	};

	if (auto sres = vkCreateSwapchainKHR(RenderDevice::get_singleton()->get_render_device().logicalDevice, &swapchain_create_info, nullptr, &m_swapchain.swapchain); sres != VK_SUCCESS)
		return false;

	uint32_t imageCount = 0;
	if (vkGetSwapchainImagesKHR(RenderDevice::get_singleton()->get_render_device().logicalDevice, m_swapchain.swapchain, &imageCount, nullptr) != VK_SUCCESS)
	{
		return false;
	}

	m_swapchain.swapchainImages.resize(imageCount);
	if (vkGetSwapchainImagesKHR(RenderDevice::get_singleton()->get_render_device().logicalDevice, m_swapchain.swapchain, &imageCount, m_swapchain.swapchainImages.data()) != VK_SUCCESS)
	{
		return false;
	}

	// Create Image View

	m_swapchain.swapchainImageViews.resize(imageCount);

	VkImageViewCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = this->m_surfaceFormat.format;

	// Default Usage 
	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;
	for (uint32_t i = 0; i < imageCount; i++)
	{

		createInfo.image = m_swapchain.swapchainImages[i];

		vkCreateImageView(RenderDevice::get_singleton()->get_render_device().logicalDevice, &createInfo, nullptr, &(m_swapchain.swapchainImageViews[i]));

	}


	// Create FrameBuffers

	m_swapchain.frameBuffers.resize(imageCount);

	//create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
	std::vector<GNF_UVec2> sizes(m_swapchain.swapchainImages.size());
	for (int i = 0; i < sizes.size(); i++)
	{
		sizes[i].x = m_surfaceExtent.width;
		sizes[i].y = m_surfaceExtent.height;
	}

	std::vector<VkClearValue> clearValues;
	clearValues.push_back({ {0.f,0.f,0.f,0.f} });

	m_renderPass.create(RenderDevice::get_singleton()->get_render_device().logicalDevice, m_swapchain.swapchainImageViews, sizes, clearValues, this->m_surfaceFormat.format,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	return true;

}

void PresentationUIManager::set_next_image()
{
	auto err = vkAcquireNextImageKHR(RenderDevice::get_singleton()->get_render_device().logicalDevice, m_swapchain.swapchain,
		UINT64_MAX, RenderDevice::get_singleton()->get_render_device().imageAcquiredSemaphore, nullptr, &m_swapchain.currentImage);
	//LoggerServer::get_singleton()->log_cout(this, "Next Image has came out", Logger::LOG_LEVEL::DEBUG);

	if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
	{
		//swapchain rebuild
		m_swapchainNeedsValidate = true;
		LoggerManager::get_singleton()->log_cout(this, "IMAGE IS BUGGY", Logger::LOG_LEVEL::ERROR);
	}

}

void PresentationUIManager::present_image()
{
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;

	presentInfo.pSwapchains = &m_swapchain.swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &RenderDevice::get_singleton()->get_render_device().renderCompleteSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &m_swapchain.currentImage;

	auto sadasdas = vkQueuePresentKHR(RenderDevice::get_singleton()->get_render_device().mainQueue, &presentInfo);
}
const VulkanRenderpass* PresentationUIManager::get_image_as_renderpass()
{
	return &m_renderPass;
}
bool PresentationUIManager::check_device(VkPhysicalDevice dev)
{
	return true;
}
bool PresentationUIManager::handle_resize()
{

	get_swap_chain_support_details(RenderDevice::get_singleton()->get_render_device().physicalDev.physicalDev, m_surface, m_swapChainDetails);

	VkSwapchainCreateInfoKHR swapchain_create_info = {
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		nullptr,
		0,
		m_surface,
		(uint32_t)m_imageCount,
		m_surfaceFormat.format,
		m_surfaceFormat.colorSpace,
		m_surfaceExtent,
		1,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT , // Image Usage
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		nullptr,
		m_swapChainDetails.capabilities.currentTransform,
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		m_presentMode,
		VK_TRUE,
		m_swapchain.swapchain
	};

	if (auto sres = vkCreateSwapchainKHR(RenderDevice::get_singleton()->get_render_device().logicalDevice, &swapchain_create_info, nullptr, &m_swapchain.swapchain); sres != VK_SUCCESS)
		return false;
	// Create Image Views

	uint32_t imageCount = 0;
	if (vkGetSwapchainImagesKHR(RenderDevice::get_singleton()->get_render_device().logicalDevice, m_swapchain.swapchain, &imageCount, nullptr) != VK_SUCCESS)
	{
		return false;
	}

	if (vkGetSwapchainImagesKHR(RenderDevice::get_singleton()->get_render_device().logicalDevice, m_swapchain.swapchain, &imageCount, m_swapchain.swapchainImages.data()) != VK_SUCCESS)
	{
		return false;
	}


	VkImageViewCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	info.format = m_surfaceFormat.format;
	info.components.r = VK_COMPONENT_SWIZZLE_R;
	info.components.g = VK_COMPONENT_SWIZZLE_G;
	info.components.b = VK_COMPONENT_SWIZZLE_B;
	info.components.a = VK_COMPONENT_SWIZZLE_A;
	VkImageSubresourceRange image_range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	info.subresourceRange = image_range;
	for (uint32_t i = 0; i < m_swapchain.swapchainImageViews.size(); i++)
	{
		vkDestroyImageView(RenderDevice::get_singleton()->get_render_device().logicalDevice, m_swapchain.swapchainImageViews[i], nullptr);

		info.image = m_swapchain.swapchainImages[i];
		
		vkCreateImageView(RenderDevice::get_singleton()->get_render_device().logicalDevice, &info, nullptr, &(m_swapchain.swapchainImageViews[i]));

	}
	m_renderPass.destroy(RenderDevice::get_singleton()->get_render_device().logicalDevice);
	std::vector<GNF_UVec2> sizes(m_swapchain.swapchainImages.size());
	for (int i = 0; i < sizes.size(); i++)
	{
		sizes[i].x = m_surfaceExtent.width;
		sizes[i].y = m_surfaceExtent.height;
	}

	std::vector<VkClearValue> clearValues;
	clearValues.push_back({ {0.f,0.f,0.f,0.f} });

	m_renderPass.create(RenderDevice::get_singleton()->get_render_device().logicalDevice, m_swapchain.swapchainImageViews, sizes, clearValues, m_surfaceFormat.format,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	m_swapchainNeedsValidate = false;
	return true;

}
bool PresentationUIManager::does_need_handle_resize()
{
	return m_swapchainNeedsValidate;
}
void PresentationUIManager::on_window_resize(const GNF_UVec2& size)
{
	m_surfaceExtent.height = size.y;
	m_surfaceExtent.width = size.x;
	m_swapchainNeedsValidate = true;
}
bool PresentationUIManager::create_surface()
{
	if (glfwCreateWindowSurface(RenderDevice::get_singleton()->get_instance().instance, (GLFWwindow*)WindowManager::get_singleton()->get_handle(), nullptr, &m_surface) != VK_SUCCESS)
		return false;
	return true;
}

void PresentationUIManager::begin_renderpass(VkCommandBuffer buff)
{
	return m_renderPass.begin(buff, m_swapchain.currentImage);
}

uint32_t PresentationUIManager::get_image_count() const noexcept
{
	return m_imageCount;
}

void PresentationUIManager::end_renderpass(VkCommandBuffer buff)
{
	return m_renderPass.end(buff);
}
