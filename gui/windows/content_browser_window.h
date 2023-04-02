#ifndef CONTENT_BROWSER_WINDOW_H
#define CONTENT_BROWSER_WINDOW_H

#include <boost/filesystem.hpp>
#include "../window.h"

class ContentBrowserWindow : public Window
{
	OBJECT_DEF(ContentBrowserWindow, Window)
public:
	ContentBrowserWindow(const String& name) : Window(name)
	{}

	virtual void render() override;
	
	bool set_path(const String& path);
	

private:
	boost::filesystem::path m_path = boost::filesystem::current_path();
};


#endif // CONTENT_BROWSER_WINDOW_H