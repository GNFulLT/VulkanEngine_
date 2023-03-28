#ifndef RENDER_SCENE_H
#define RENDER_SCENE_H

#include "../../core/object/object.h"
#include "../../core/vec2.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

#include "../../graphic/vulkan_renderpass.h"

class RenderScene : public Object
{
	OBJECT_DEF(RenderScene,Object)
public:
	RenderScene();
	~RenderScene();

	//X Needed
	bool init(const GNF_UVec2& initial_size);
	void destroy();
	void on_resize(const GNF_UVec2& size);
	void render_ex(VkQueue queue,VkCommandBuffer buff, VkFence fence);
	bool fill_cmd(VkCommandBuffer buff);
	_F_INLINE_ VkDescriptorSet get_image() const noexcept
	{
		return m_descriptor_set;
	}
private:
	GNF_UVec2 m_size;
	VkDevice m_dev;
	VkPhysicalDevice m_physical_dev;
	VkAllocationCallbacks m_callbacks;

	// Source
	VkImage m_image = nullptr;
	VkDeviceMemory  m_image_memory = nullptr;
	VkImageView m_image_view = nullptr;
	VkSampler m_image_sampler = nullptr;
	VkDescriptorSet m_descriptor_set = nullptr;

	VulkanRenderpass m_renderPass;

	// Render Target
	//VkRenderPass m_render_pass = nullptr;
	//VkFramebuffer m_frame_buffer = nullptr;
private:
	bool isViewDirty = false;
	_INLINE_ VkCommandBufferBeginInfo* get_main_begin_inf()
	{
		static VkCommandBufferBeginInfo beginInf{
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			nullptr,
			0,
			nullptr
		};

		return &beginInf;
	}
};


#endif // RENDER_SCENE_H