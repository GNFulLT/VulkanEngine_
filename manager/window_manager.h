#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include <unordered_map>
#include <utility>
#include <concepts>

#include "../core/vec2.h"
#include "../config/config_property.h"
#include "../core/typedefs.h"
#include "system_manager.h"
#include "../gui/window.h"
#include <GLFW/glfw3.h>

template<typename T>
concept window_type = std::convertible_to<T*,Window*>;

class WindowManager : public SystemManager
{
	SINGLETON(WindowManager)
	OBJECT_DEF(WindowManager, SystemManager)
public:
	WindowManager();

	void destroy() override;

	enum WINDOW_MODE
	{
		WINDOW_MODE_WINDOWED = 0,
		WINDOW_MODE_MINIMIZED,
		WINDOW_MODE_MAXIMIZED,
		WINDOW_MODE_FULLSCREEN
	};

	bool init() override final;

	bool need_render();

	_F_INLINE_ GLFWwindow* get_window()
	{
		return m_window;
	}

	void on_resize(int width, int height);
	void on_iconify_changed(int iconified);
	
	template<window_type T,typename... Args>
	bool create_window(String name,T** ppWindow,Args... args)
	{
		if (m_createdWindows.find(name) != m_createdWindows.end())
		{
			return false;
		}
		auto pWindow = MemoryManager::get_singleton()->new_object<T>(name, name,args...);
		m_createdWindows.emplace(name, pWindow);
		*ppWindow = pWindow;
		return true;
	}

	template<window_type T>
	void destroy_window(T* ptr)
	{
		if (m_createdWindows.find(ptr->get_name()) == m_createdWindows.end())
			return;
		m_createdWindows.erase(ptr->get_name());
		MemoryManager::get_singleton()->destroy_object<T>(ptr);
	}

	_INLINE_ Window* get_registered_window(const String& str)
	{
		return m_registeredWindows.find(str).operator->()->second.second;
	}
	
	_INLINE_ bool register_window(Window* window)
	{
		m_registeredWindows.emplace(window->get_name(),std::pair(true,window));
		m_registeredWindowsList.push_back(window);
		return true;
	}

	_INLINE_ void show()
	{
		assert(m_window != nullptr);


		glfwShowWindow(m_window);
	}

	_INLINE_ bool wants_close()
	{
		return glfwWindowShouldClose(m_window);
	}

	_INLINE_ void handle_window_events()
	{
		glfwPollEvents();
	}

	_INLINE_ const GNF_UVec2* get_size_r() const noexcept
	{
		return m_size.get();
	}

	_INLINE_ const ConfigProperty<GNF_UVec2>* get_size() const noexcept
	{
		return &m_size;
	}

	void pre_render();
	void render();
	void on_created();
private:
	GLFWwindow* m_window = nullptr;
	GLFWmonitor* m_monitor = nullptr;
private:
	ConfigProperty<GNF_UVec2> m_size;
	ConfigProperty<WINDOW_MODE> m_windowMode;
	WINDOW_MODE m_lastMode;
private:
	std::vector<Window*> m_registeredWindowsList;
	std::unordered_map<String,Window*> m_createdWindows;
	std::unordered_map<String,std::pair<bool,Window*>> m_registeredWindows;
	unsigned int m_dock_id;
};


#endif // WINDOW_MANAGER_H