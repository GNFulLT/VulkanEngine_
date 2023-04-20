#ifndef PRESENTATION_UI_MANAGER_H
#define PRESENTATION_UI_MANAGER_H

#include "presentation_manager.h"
#include "../../graphic/vulkan_renderpass.h"
#include "../../core/utils_vulkan.h"
#include "../../core/vec2.h"
#include "../../core/typedefs.h"

#include <boost/signals2.hpp>

struct VulkanSwapchain
{
	VkSwapchainKHR swapchain = nullptr;

	//X This is images created with swapchain
	std::vector<VkImage> swapchainImages;

	//X This is image views for swapchainImages to able writing into image
	std::vector<VkImageView> swapchainImageViews;

	//X This is to make able imageview be targeted as a frame buffer by presentation
	std::vector<VkFramebuffer> frameBuffers;

	uint32_t currentImage = 0;
};


class PresentationUIManager : public PresentationManager
{
	OBJECT_DEF(PresentationUIManager,PresentationManager)
public:
	PresentationUIManager();
	bool init() override;

	void set_next_image() override;

	void present_image() override;

	const VulkanRenderpass* get_image_as_renderpass() override;
	
	bool check_device(VkPhysicalDevice dev) override;

	bool handle_resize() override;

	bool does_need_handle_resize() override;

	_F_INLINE_ uint32_t get_image_index() const noexcept
	{
		return m_swapchain.currentImage;
	}

	_F_INLINE_ void begin_renderpass(VkCommandBuffer buff) _NO_EXCEPT_
	{
		return m_renderPass.begin(buff,m_swapchain.currentImage);
	}

	uint32_t get_image_count() const noexcept override;

	_F_INLINE_ void end_renderpass(VkCommandBuffer buff) _NO_EXCEPT_
	{
		return m_renderPass.end(buff);
	}


protected:

	void on_window_resize(const GNF_UVec2& size);
private:
	bool create_surface();

	//X Swapchain struct
private:
	VulkanSwapchain m_swapchain;
private:
	//X Directly related to swapchain
	VkSurfaceKHR m_surface;
	VkSurfaceFormatKHR m_surfaceFormat;
	VkPresentModeKHR m_presentMode;
	int m_imageCount;
	VkExtent2D m_surfaceExtent;
	//X Cached
private:
	SwapChainSupportDetails m_swapChainDetails;
	bool m_swapchainNeedsValidate = false;
private:
	VulkanRenderpass m_renderPass;
	boost::signals2::connection m_windowResizeConnection;
};


#endif // PRESENTATION_UI_MANAGER_H