#include  "editor_client_window.h"
#include "../main/application.h"
EditorClientWindow::EditorClientWindow()
{
}

void EditorClientWindow::destroy()
{
	SystemManager::destroy();
}

bool EditorClientWindow::init()
{
	return true;
}

bool EditorClientWindow::need_render()
{
	return m_needRender;
}

void* EditorClientWindow::get_handle()
{
	return nullptr;
}

void EditorClientWindow::show()
{
}

bool EditorClientWindow::wants_close()
{
	return m_wantsClose;
}

void EditorClientWindow::handle_window_events()
{
	if (m_wantsClose)
		Application::exit();
}

const GNF_UVec2* EditorClientWindow::get_size_r() const noexcept
{
	return m_size.get();
}

const ConfigProperty<GNF_UVec2>* EditorClientWindow::get_size() const noexcept
{
	return &m_size;
}
