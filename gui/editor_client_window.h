#ifndef EDITOR_CLIENT_WINDOW_H
#define EDITOR_CLIENT_WINDOW_H

#include "../manager/window_manager.h"


class EditorClientWindow : public WindowManager
{
	OBJECT_DEF(EditorClientWindow,WindowManager)
public:
	EditorClientWindow();
	virtual void destroy() override;

	virtual bool init() override;

	virtual bool need_render() override;

	virtual void* get_handle() override;

	virtual void show() override;


	virtual bool wants_close() override;


	virtual void handle_window_events() override;


	virtual const GNF_UVec2* get_size_r() const noexcept override;


	virtual const ConfigProperty<GNF_UVec2>* get_size() const noexcept override;
private:
	ConfigProperty<GNF_UVec2> m_size;
	bool m_needRender = true;
	bool m_wantsClose = false;
};

#endif // NON_WINDOW_MANAGER_H