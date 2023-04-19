#ifndef RENDER_SCENE_UI_H
#define RENDER_SCENE_UI_H

#include "render_scene.h"

class RenderSceneUI : public RenderScene
{
	OBJECT_DEF(RenderSceneUI,RenderScene)
public:
	virtual bool fill_cmd(VkCommandBuffer buff) override;
private:
};


#endif // RENDER_SCENE_UI_H