#include "render_scene_ui.h"
#include "render_device.h"
bool RenderSceneUI::fill_cmd(VkCommandBuffer buff)
{

	auto res = vkBeginCommandBuffer(buff, get_main_begin_inf());
	m_renderPass->begin(buff,*RenderDevice::get_singleton()->get_current_image());

	m_renderPass->end(buff);
	res = vkEndCommandBuffer(buff);

	return true;
}
