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
	virtual ~WindowManager() = default;
	WindowManager() = default;

	virtual void destroy() = 0;
	enum WINDOW_MODE
	{
		WINDOW_MODE_WINDOWED = 0,
		WINDOW_MODE_MINIMIZED,
		WINDOW_MODE_MAXIMIZED,
		WINDOW_MODE_FULLSCREEN
	};

	virtual bool init() = 0;

	virtual bool need_render() = 0;

	virtual void* get_handle() = 0;
	
	virtual void show() = 0;
	

	virtual bool wants_close() = 0;


	virtual void handle_window_events() = 0;


	virtual const GNF_UVec2* get_size_r() const noexcept = 0;


	virtual const ConfigProperty<GNF_UVec2>* get_size() const noexcept = 0;
};


#endif // WINDOW_MANAGER_H