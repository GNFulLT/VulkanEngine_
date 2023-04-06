#ifndef CONTENT_BROWSER_WINDOW_H
#define CONTENT_BROWSER_WINDOW_H

#include <boost/filesystem.hpp>
#include "../window.h"
#include "../../core/io/resource.h"
#include "../../core/typedefs.h"


class ContentBrowserWindow : public Window
{
	OBJECT_DEF(ContentBrowserWindow, Window)
public:
	ContentBrowserWindow(const String& name) : Window(name)
	{}
	~ContentBrowserWindow();
	virtual void render() override;
	virtual bool on_created() override final;
	bool set_path(const String& path);
	

private:
	boost::filesystem::path m_path = boost::filesystem::current_path();
	Resource* m_textIcon = nullptr;
	Resource* m_folderIcon = nullptr;
};


#endif // CONTENT_BROWSER_WINDOW_H