#ifndef WINDOW_H
#define WINDOW_H

#include <concepts>

#include "../core/object/object.h"
#include "../config/config_property.h"
#include "../imgui/imgui.h"
#include "../core/string/string_utils.h"
class WindowManager;


class Window : public Object
{
	OBJECT_DEF(Window,Object)
public:
	Window(const String& name) : m_size(this,GNF_UVec2{.x=0,.y=0}, "Size")
	{
		m_windowName = name;
		m_id = hash_string(name);
	}

	virtual void on_size_changed();

	_INLINE_ bool check_size_changed() const noexcept
	{
		auto size = ImGui::GetContentRegionAvail();
		if (size.x == m_size.get()->x && size.y == m_size.get()->y)
			return false;
		return true;
	}

	_INLINE_ const GNF_UVec2* get_size_r() const noexcept
	{
		return m_size.get();
	}

	_INLINE_ const ConfigProperty<GNF_UVec2>* get_size() const noexcept
	{
		return &m_size;
	}

	virtual bool need_render() const noexcept
	{
		return true;
	}

	_INLINE_ const String& get_name() const noexcept
	{
		return m_windowName;
	}

	_INLINE_ size_t get_window_id() const noexcept
	{
		return m_id;
	}

public:
	virtual void render() = 0;
	virtual void pre_render();
	virtual bool on_created();

protected:
	ConfigProperty<GNF_UVec2> m_size;
private:
	friend class WindowManager;
	size_t m_id;
	String m_windowName;
};

#endif // WINDOW_H