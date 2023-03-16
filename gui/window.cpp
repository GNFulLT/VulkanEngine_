#include "window.h"

#include "../imgui/imgui.h"

void Window::pre_render()
{
	auto size = ImGui::GetWindowSize();
	this->m_size.set(GNF_UVec2(size.x,size.y));
}

void Window::on_created()
{

}