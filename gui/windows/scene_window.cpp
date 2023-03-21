#include "scene_window.h"

#include "../../manager/memory_manager.h"
#include "../../manager/rendering/render_device.h"
#include <boost/bind/bind.hpp>

SceneWindow::SceneWindow(String name, RenderScene* pRenderScene) : Window(name), p_render_scene(pRenderScene)
{
	m_quittingConnection = RenderDevice::get_singleton()->get_quitting()->subscribe_changed_event(boost::bind(&SceneWindow::on_device_destroyed,this,boost::placeholders::_1));
}

void SceneWindow::render()
{

}

bool SceneWindow::on_created()
{
	return p_render_scene->init(*m_size.get());
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