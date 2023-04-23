#ifndef GLFW_WINDOW_H
#define GLFW_WINDOW_H

#include "window_manager.h"


class GlfwWindowManager : public WindowManager
{
	OBJECT_DEF(GlfwWindowManager, WindowManager)

public:
	GlfwWindowManager();

	virtual void destroy() override;

	virtual bool init() override;

	virtual bool need_render() override;

	virtual void* get_handle() override;


	void on_resize(int width, int height);
	void on_iconify_changed(int iconified);

	virtual void show() override;


	virtual bool wants_close() override;


	virtual void handle_window_events() override;


	virtual const GNF_UVec2* get_size_r() const noexcept;


	virtual const ConfigProperty<GNF_UVec2>* get_size() const noexcept;


	_INLINE_ const ConfigProperty<WINDOW_MODE>* get_window() const noexcept
	{
		return &m_windowMode;
	}

private:
	GLFWwindow* m_window = nullptr;
	GLFWmonitor* m_monitor = nullptr;
private:
	ConfigProperty<GNF_UVec2> m_size;
	ConfigProperty<WINDOW_MODE> m_windowMode;
	WINDOW_MODE m_lastMode;
};


#endif //GLFW_WINDOW_H