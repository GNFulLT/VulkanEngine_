#ifndef RENDER_SCENE_H
#define RENDER_SCENE_H

#include "../../core/object/object.h"
#include "../../core/vec2.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

class RenderScene : public Object
{
	OBJECT_DEF(RenderScene,Object)
public:
	RenderScene();
	~RenderScene();

	//X Needed
	bool init(const GNF_UVec2& initial_size);

	void on_resize(const GNF_UVec2& size);

	void render_ex(VkQueue queue,VkCommandBuffer buff, VkFence fence);

	_F_INLINE_ VkDescriptorSet get_image() const noexcept
	{
		return m_descriptor_set;
	}
private:
	GNF_UVec2 m_size;
	VkDevice m_dev;
	VkPhysicalDevice m_physical_dev;
	VkAllocationCallbacks m_callbacks ;

	// Source
	VkImage m_image = nullptr;
	VkDeviceMemory  m_image_memory = nullptr;
	VkImageView m_image_view = nullptr;
	VkSampler m_image_sampler = nullptr;
	VkDescriptorSet m_descriptor_set = nullptr;

	// Render Target
	VkRenderPass m_render_pass = nullptr;
	VkFramebuffer m_frame_buffer = nullptr;
private:
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

	_INLINE_ VkRenderPassBeginInfo* get_main_renderpass_begin_inf()
	{
		static VkClearValue clearValue
		{
			 { { 0.0f, 1.0f, 0.0f, 1.0f } }
		};

		static VkRenderPassBeginInfo beginInf{
			VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			nullptr,
			m_render_pass,
			m_frame_buffer,
			{{(int32_t)0,(int32_t)0},{m_size.x,m_size.y}},	
			1,
			&clearValue
		};

		beginInf.renderArea.extent.width = m_size.x;
		beginInf.renderArea.extent.height = m_size.y;

		return &beginInf;
	}
};


#endif // RENDER_SCENE_H