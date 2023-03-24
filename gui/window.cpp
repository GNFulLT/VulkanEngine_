#include "window.h"

#include "../imgui/imgui.h"

void Window::on_size_changed()
{
	
}

void Window::pre_render()
{
	auto size = ImGui::GetWindowSize();
	GNF_UVec2 vec;
	vec.x = size.x;
	vec.y = size.y;
	this->m_size.set(vec);
}

bool Window::on_created()
{
	return true;
}