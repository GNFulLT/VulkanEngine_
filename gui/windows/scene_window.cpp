#include "scene_window.h"

#include "../../manager/memory_manager.h"
#include "../../manager/rendering/render_device.h"
#include "../../imgui/imgui.h"
#include <boost/bind/bind.hpp>

SceneWindow::SceneWindow(String name, RenderScene* pRenderScene) : Window(name), p_render_scene(pRenderScene)
{
	m_quittingConnection = RenderDevice::get_singleton()->get_quitting()->subscribe_changed_event(boost::bind(&SceneWindow::on_device_destroyed,this,boost::placeholders::_1));
}

void SceneWindow::render()
{
	ImGui::Image(p_render_scene->get_image(),ImVec2(1920,1080));
}

bool SceneWindow::on_created()
{
	if (p_render_scene == nullptr)
		return false;
	GNF_UVec2 size = {};
	size.x = 1920;
	size.y = 1080;
	return p_render_scene->init(size);
}

void SceneWindow::render_ex(VkQueue queue,VkCommandBuffer buff,VkFence fence)
{
	p_render_scene->render_ex(queue,buff,fence);
}

void SceneWindow::on_device_destroyed(const bool& quitting)
{
	if (quitting)
	{
		MemoryManager::get_singleton()->destroy_object(p_render_scene);
		p_render_scene = nullptr;
	}
}

SceneWindow::~SceneWindow()
{
	if(p_render_scene != nullptr)
		MemoryManager::get_singleton()->destroy_object(p_render_scene);

}