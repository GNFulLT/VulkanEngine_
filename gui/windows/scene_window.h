#ifndef SCENE_WINDOW_H
#define SCENE_WINDOW_H

#include "../window.h"
#include "../../manager/rendering/render_scene.h"

class SceneWindow : public Window
{
	OBJECT_DEF(SceneWindow,Window)
public:
	SceneWindow(String name, RenderScene* pRenderScene);
	~SceneWindow();
public:
	inline RenderScene* get_render_scene() const noexcept
	{
		return p_render_scene;
	}
	void delete_buff();
	void render() override final;
	bool on_created() override final;
	void render_ex(VkQueue queue, VkCommandBuffer buff, VkFence fence);
	void pre_render() override final;
	uint8_t* get_image_bytes();
	void on_size_changed() override final;
private:
private:
	RenderScene* p_render_scene;
};

#endif // SCENE_WINDOW_H

