#ifndef SCENE_WINDOW_H
#define SCENE_WINDOW_H

#include "../window.h"
#include "../../manager/rendering/render_scene.h"
#include <boost/signals2.hpp>

class SceneWindow : public Window
{
	OBJECT_DEF(SceneWindow,Window)
public:
	SceneWindow(String name, RenderScene* pRenderScene);
	~SceneWindow();
public:
	void render() override final;
	bool on_created() override final;
	void render_ex(VkQueue queue, VkCommandBuffer buff, VkFence fence);
	void pre_render() override final;

	void on_size_changed() override final;
private:
	void on_device_destroyed(const bool& quitting);

	boost::signals2::connection m_quittingConnection;
private:
	RenderScene* p_render_scene;
};

#endif // SCENE_WINDOW_H

