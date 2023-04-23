#include "glfw_window.h"

#include "memory_manager.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "rendering/render_device.h"
#include "logger_manager.h"
#include "rendering/render_scene.h"
#include "../main/application.h"

#include <boost/format.hpp>

GlfwWindowManager::GlfwWindowManager() : m_lastMode(WINDOW_MODE_WINDOWED), m_windowMode(this, WindowManager::WINDOW_MODE_WINDOWED, "WindowMode"), m_size(this, GNF_UVec2{ .x = 640, .y = 480 }, "Size")
{
}
//
//bool WindowManager::need_render(const String& windowName)
//{
//	auto win = m_registeredWindows.find(windowName).operator->();
//	return need_render() && win->second.first && win->second.second->need_render();
//}

void* GlfwWindowManager::get_handle()
{
		return m_window;
}

bool GlfwWindowManager::init()
{

	if (!glfwInit())
		return false;

	/*switch (*m_monitorSelection->get_prop())
	{
	case MONITOR_SELECTION_PRIMARY:
		m_monitor = nullptr;
		break;
	default:
		m_monitor = nullptr;
		break;
	}*/

	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	// Vulkan
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	switch (*m_windowMode.get())
	{
	case WINDOW_MODE_WINDOWED:
		m_window = glfwCreateWindow(m_size.get()->x, m_size.get()->y, "GNF Engine", m_monitor, NULL);
		break;
		// Not Supported Yet
	default:
		m_window = glfwCreateWindow(m_size.get()->x, m_size.get()->y, "GNF Engine", m_monitor, NULL);
		break;
	}

	glfwSetWindowUserPointer(m_window, this);


	auto resizeCallback = [](GLFWwindow* window, int width, int height) {
		((GlfwWindowManager*)glfwGetWindowUserPointer(window))->on_resize(width, height);

	};

	auto iconifyCallback = [](GLFWwindow* window, int iconified)
	{
		((GlfwWindowManager*)glfwGetWindowUserPointer(window))->on_iconify_changed(iconified);

	};

	glfwSetWindowIconifyCallback(m_window, iconifyCallback);
	glfwSetWindowSizeCallback(m_window, resizeCallback);

	return true;
}

void GlfwWindowManager::on_iconify_changed(int iconified)
{
	if (iconified)
	{
		m_lastMode = *m_windowMode.get();
		m_windowMode.set(WINDOW_MODE_MINIMIZED);
	}
	else
	{
		m_windowMode.set(m_lastMode);
	}
}

void GlfwWindowManager::destroy()
{
	//size_t i = m_createdWindows.size();
	//auto windowIterator = m_createdWindows.begin();
	//while (windowIterator != m_createdWindows.end())
	//{
	//	destroy_window(windowIterator->second);
	//	windowIterator = m_createdWindows.begin();
	//}
	SystemManager::destroy();
}
//
//void WindowManager::rebuild_window()
//{
//	ImGui::DockBuilderRemoveNode(m_dock_id);
//
//	ImGui::DockBuilderAddNode(m_dock_id, ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace);
//	ImGui::DockBuilderSetNodeSize(m_dock_id, ImGui::GetMainViewport()->Size);
//
//	ImGuiID dock_id_left_menu;
//
//	auto dock_id_rigt_menu = ImGui::DockBuilderSplitNode(m_dock_id, ImGuiDir_Right,
//		0.3f, nullptr, &dock_id_left_menu);
//
//	ImGuiID dock_id_middle;
//
//	auto dock_id_left = ImGui::DockBuilderSplitNode(dock_id_left_menu, ImGuiDir_Left,
//		0.2f, nullptr, &dock_id_middle);
//	
//	
//	auto dock_id_bottom = ImGui::DockBuilderSplitNode(dock_id_middle, ImGuiDir_Down,
//		0.2f, nullptr, &dock_id_middle);
//
//	ImGui::DockBuilderDockWindow("Dear ImGui Demo", dock_id_left);
//	ImGui::DockBuilderDockWindow("Text Editor", dock_id_rigt_menu);
//	ImGui::DockBuilderDockWindow("Scene", dock_id_middle);
//	ImGui::DockBuilderDockWindow("Content Browser", dock_id_bottom);
//
//	ImGui::DockBuilderFinish(m_dock_id);
//}

void GlfwWindowManager::show()
{
	
	assert(m_window != nullptr);


	glfwShowWindow(m_window);
	
}

bool GlfwWindowManager::wants_close()
{
	return glfwWindowShouldClose(m_window);
}

void GlfwWindowManager::handle_window_events()
{
	glfwPollEvents();
	if (wants_close())
	{
		Application::exit();
	}
}

const GNF_UVec2* GlfwWindowManager::get_size_r() const noexcept
{
	return m_size.get();
}

const ConfigProperty<GNF_UVec2>* GlfwWindowManager::get_size() const noexcept
{
	return &m_size;
}

//void WindowManager::pre_render()
//{
//	
//
//	if (ImGui::BeginMainMenuBar())
//	{
//		
//		
//		if (ImGui::BeginMenu("View"))
//		{
//			for (int i = 0; i < m_registeredWindowsList.size()-1; i++)
//			{
//				ImGui::Checkbox(m_registeredWindowsList[i]->get_name().c_str(), &m_registeredWindows[m_registeredWindowsList[i]->get_name()].first);
//				ImGui::Separator();
//			}
//			ImGui::Checkbox(m_registeredWindowsList[m_registeredWindowsList.size() - 1]->get_name().c_str(), &m_registeredWindows[m_registeredWindowsList[m_registeredWindowsList.size() - 1]->get_name()].first);
//
//			ImGui::EndMenu();
//		}
//		ImGui::EndMainMenuBar();
//	}
//
//	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
//	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
//	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
//	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
//	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
//	ImGuiViewport* viewport = ImGui::GetMainViewport();
//	ImGui::SetNextWindowPos(viewport->WorkPos);
//	ImGui::SetNextWindowSize(viewport->WorkSize);
//	ImGui::SetNextWindowViewport(viewport->ID);
//	ImGui::Begin("DockSpace", nullptr, window_flags);
//	m_dock_id = ImGui::GetID("##MainDocker");
//	ImGui::PopStyleVar(2);
//
//	ImGui::DockSpace(m_dock_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_::ImGuiDockNodeFlags_NoResize
//		| ImGuiDockNodeFlags_PassthruCentralNode);
//
//
//	ImGui::ShowDemoWindow();
//
//
//	for (auto& window : m_registeredWindows)
//	{
//		if (window.second.first && window.second.second->need_render())
//		{
//			if (ImGui::Begin(window.second.second->get_name().c_str()))
//			{
//				window.second.second->pre_render();
//				ImGui::End();
//			}
//		}
//	}
//
//	ImGui::End();
//
//}
//
//void WindowManager::render()
//{
//	bool needValidation = false;
//	// Main Menu
//	if (ImGui::BeginMainMenuBar())
//	{
//		ImVec4 col = { 0,0,0,0 };
//		ImGui::PushStyleColor(ImGuiCol_FrameBg, col);
//		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, col);
//		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, col);
//		if (ImGui::BeginMenu("View"))
//		{
//			for (int i = 0; i < m_registeredWindowsList.size() - 1; i++)
//			{
//				if (ImGui::Checkbox(m_registeredWindowsList[i]->get_name().c_str(), &m_registeredWindows[m_registeredWindowsList[i]->get_name()].first))
//				{
//					needValidation = true;
//				}
//				ImGui::Separator();
//			}
//			if (ImGui::Checkbox(m_registeredWindowsList[m_registeredWindowsList.size() - 1]->get_name().c_str(), &m_registeredWindows[m_registeredWindowsList[m_registeredWindowsList.size() - 1]->get_name()].first))
//			{
//				needValidation = true;
//			}
//
//			ImGui::EndMenu();
//		}
//		ImGui::PopStyleColor(3);
//
//		ImGui::EndMainMenuBar();
//	}
//
//	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
//	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
//	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
//	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
//	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
//	ImGuiViewport* viewport = ImGui::GetMainViewport();
//	ImGui::SetNextWindowPos(viewport->WorkPos);
//	ImGui::SetNextWindowSize(viewport->WorkSize);
//	ImGui::SetNextWindowViewport(viewport->ID);
//	ImGui::Begin("DockSpace", nullptr, window_flags);
//	m_dock_id = ImGui::GetID("##MainDocker");
//	ImGui::PopStyleVar(2);
//
//	ImGui::DockSpace(m_dock_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_::ImGuiDockNodeFlags_NoResize
//		| ImGuiDockNodeFlags_PassthruCentralNode);
//
//
//
//	ImGui::ShowDemoWindow();
//	for (auto& window : m_registeredWindows)
//	{
//		if (window.second.first && window.second.second->need_render())
//		{
//			if (ImGui::Begin(window.second.second->get_name().c_str()))
//			{
//				window.second.second->render();
//			}
//			ImGui::End();
//		}
//	}
//	ImGui::End();
//	m_needValidation = needValidation;
//}
//
//void WindowManager::on_created()
//{
//	
//	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
//	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
//	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
//	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
//	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
//	ImGuiViewport* viewport = ImGui::GetMainViewport();
//	ImGui::SetNextWindowPos(viewport->WorkPos);
//	ImGui::SetNextWindowSize(viewport->WorkSize);
//	ImGui::SetNextWindowViewport(viewport->ID);
//	ImGui::Begin("DockSpace", nullptr, window_flags);
//	m_dock_id = ImGui::GetID("##MainDocker");
//	ImGui::PopStyleVar(2);
//
//
//	ImGui::DockSpace(m_dock_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_::ImGuiDockNodeFlags_NoResize
//		| ImGuiDockNodeFlags_PassthruCentralNode);
//
//	rebuild_window();
//
//	ImGui::End();
//	
//
//	auto iterator = m_registeredWindowsList.begin();
//
//	while (iterator != m_registeredWindowsList.end())
//	{
//		bool created = (*iterator.operator->())->on_created();
//		if (!created)
//		{
//			LoggerManager::get_singleton()->log_cout(this, boost::str(boost::format("Detach %1 Window because it couldn't be created") % (*iterator.operator->())->get_name()).c_str(),Logger::WARNING);
//			iterator = m_registeredWindowsList.erase(iterator);
//		}
//		else
//		{
//			iterator++;
//		}
//	}
//
//}

void GlfwWindowManager::on_resize(int width,int height)
{
	GNF_UVec2 vec;
	vec.x = width;
	vec.y = height;
	m_size.set(vec);
	if (width == 0 && height == 0)
	{
		m_windowMode.set(WINDOW_MODE_MINIMIZED);
	}
	else
	{
		m_windowMode.set(WINDOW_MODE_WINDOWED);
	}
}

bool GlfwWindowManager::need_render()
{
	return *m_windowMode.get() != WINDOW_MODE_MINIMIZED;
}