#ifndef WINDOW_H
#define WINDOW_H

#include <concepts>

#include "../core/object/object.h"
#include "../config/config_property.h"

class WindowManager;


class Window : public Object
{
	OBJECT_DEF(Window,Object)
public:
	Window(const String& name) : m_size(this,GNF_UVec2(0,0),"Size")
	{
		m_windowName = name;
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

	_INLINE_ const String& get_name()
	{
		return m_windowName;
	}

	_INLINE_ unsigned int get_window_id()
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
	unsigned int m_id;
	String m_windowName;
};

#endif // WINDOW_H