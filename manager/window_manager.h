#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include <unordered_map>
#include <utility>
#include <concepts>

#include "../core/vec2.h"
#include "../config/config_property.h"
#include "../core/typedefs.h"
#include "system_manager.h"
#include <GLFW/glfw3.h>

class WindowManager : public SystemManager
{
	SINGLETON(WindowManager)
	OBJECT_DEF(WindowManager, SystemManager)
public:
	WindowManager();
	bool layout_changed = false;
	void destroy() override;
	enum WINDOW_MODE
	{
		WINDOW_MODE_WINDOWED = 0,
		WINDOW_MODE_MINIMIZED,
		WINDOW_MODE_MAXIMIZED,
		WINDOW_MODE_FULLSCREEN
	};

	virtual bool init() override;

	bool need_render();

	virtual void* get_handle();
	

	void on_resize(int width, int height);
	void on_iconify_changed(int iconified);
	
	
	_INLINE_ bool need_validation() const noexcept
	{
		return m_needValidation;
	}

	virtual void show();
	

	virtual bool wants_close();


	virtual void handle_window_events();

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
	bool m_needValidation = false;
};


#endif // WINDOW_MANAGER_H