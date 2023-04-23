#ifndef PRESENTATION_MANAGER_H
#define PRESENTATION_MANAGER_H

#include "../system_manager.h"
#include "../../graphic/vulkan_renderpass.h"


class PresentationManager : public SystemManager
{
	OBJECT_DEF(PresentationManager,SystemManager)
	SINGLETON(PresentationManager)
public:
	PresentationManager() = default;
	virtual ~PresentationManager() = default;
	virtual bool init() = 0;

	virtual void set_next_image() = 0;
	
	virtual void present_image() = 0;
	
	virtual const VulkanRenderpass* get_image_as_renderpass() = 0;

	virtual bool handle_resize() = 0;

	virtual bool does_need_handle_resize() = 0;

	virtual void begin_renderpass(VkCommandBuffer buff) = 0;
	virtual void end_renderpass(VkCommandBuffer buff) = 0;


	//X Calls before init
	virtual bool check_device(VkPhysicalDevice dev) = 0;

	virtual uint32_t get_image_count() const noexcept = 0;
private:
};


#endif // PRESENTATION_MANAGER_H