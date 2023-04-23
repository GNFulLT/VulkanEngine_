#include "render_scene_ui.h"
#include "render_device.h"
#include "presentation_manager.h"
bool RenderSceneUI::fill_cmd(VkCommandBuffer buff)
{

	auto res = vkBeginCommandBuffer(buff, get_main_begin_inf());
	(PresentationManager::get_singleton())->begin_renderpass(buff);

	(PresentationManager::get_singleton())->end_renderpass(buff);
	res = vkEndCommandBuffer(buff);

	return true;
}
