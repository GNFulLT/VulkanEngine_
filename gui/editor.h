#ifndef NO_USE_EDITOR

#ifndef EDITOR_H
#define EDITOR_H


#include "../main/application.h"


class Editor : public Application
{
public:
	Editor();
	virtual ~Editor() = default;
	virtual int before_start_impl() override;
	virtual void update_impl() override;
	virtual void render_impl() override;
	virtual bool begin_frame_impl() override;

	virtual void fixed_update_impl() override;
	virtual void before_exit_impl() override;
protected:
	virtual void render_scene_impl() override; 
	virtual void override_loaders() override;
	
private:
	void set_next_image_e();
	void present_image_e();
	void process_events_e();

	GNF_UVec2 m_offscreenSize;
};


#endif // EDITOR_H
#endif // NO_USE_EDITOR
