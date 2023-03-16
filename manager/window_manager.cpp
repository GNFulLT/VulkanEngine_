#include "window_manager.h"

#include "memory_manager.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
WindowManager::WindowManager() : m_lastMode(WINDOW_MODE_WINDOWED), m_windowMode(this, WindowManager::WINDOW_MODE_WINDOWED, "WindowMode"), m_size(this, GNF_UVec2(640, 480), "Size")
{
	m_dock_id = 0;
}

bool WindowManager::init()
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
		((WindowManager*)glfwGetWindowUserPointer(window))->on_resize(width, height);

	};

	auto iconifyCallback = [](GLFWwindow* window, int iconified)
	{
		((WindowManager*)glfwGetWindowUserPointer(window))->on_iconify_changed(iconified);

	};

	glfwSetWindowIconifyCallback(m_window, iconifyCallback);
	glfwSetWindowSizeCallback(m_window, resizeCallback);

	return true;
}

void WindowManager::on_iconify_changed(int iconified)
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

void WindowManager::destroy()
{
	int i = m_createdWindows.size();
	auto windowIterator = m_createdWindows.begin();
	while (windowIterator != m_createdWindows.end())
	{
		destroy_window(windowIterator->second);
		windowIterator = m_createdWindows.begin();
	}

	SystemManager::destroy();
}

void WindowManager::pre_render()
{
	ImGui::DockBuilderRemoveNode(m_dock_id);

	ImGui::DockBuilderAddNode(m_dock_id, ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize(m_dock_id, ImGui::GetMainViewport()->Size);

	//ImGuiID center = m_dockId;	

	ImGuiID dock_id_left_menu;

	auto dock_id_rigt_menu = ImGui::DockBuilderSplitNode(m_dock_id, ImGuiDir_Right,
		0.3f, nullptr, &dock_id_left_menu);

	auto dock_id_left = ImGui::DockBuilderSplitNode(dock_id_left_menu, ImGuiDir_Left,
		0.2f, nullptr, &dock_id_left_menu);

	ImGui::DockBuilderDockWindow("Dear ImGui Demo", dock_id_left);
	ImGui::DockBuilderDockWindow("Zort", dock_id_rigt_menu);


	if (ImGui::BeginMainMenuBar())
	{
		ImGui::EndMainMenuBar();
	}
	
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::Begin("DockSpace", nullptr, window_flags);
	ImGuiID m_dockId = ImGui::GetID("##MainDocker");
	ImGui::PopStyleVar(2);

	ImGui::DockSpace(m_dockId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_::ImGuiDockNodeFlags_NoResize
		| ImGuiDockNodeFlags_PassthruCentralNode);


	ImGui::ShowDemoWindow();

	for (auto& window : m_registeredWindows)
	{
		if (ImGui::Begin(window.second.second->get_name().c_str()))
		{
			window.second.second->pre_render();
		}
		ImGui::End();
	}

	ImGui::End();

}

void WindowManager::render()
{
	// Main Menu
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("View"))
		{
			/*for (auto& pair : m_registeredWindows)
			{
				ImGui::Checkbox(pair.second.first->get_window_name().c_str(), &pair.second.second);
			}*/

			ImGui::EndMenu();
		}


		ImGui::EndMainMenuBar();
	}

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::Begin("DockSpace", nullptr, window_flags);
	ImGuiID m_dockId = ImGui::GetID("##MainDocker");
	ImGui::PopStyleVar(2);

	ImGui::DockSpace(m_dockId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_::ImGuiDockNodeFlags_NoResize
		| ImGuiDockNodeFlags_PassthruCentralNode);



	ImGui::ShowDemoWindow();


	for (auto& window : m_registeredWindows)
	{
		if (window.second.second->need_render())
		{
			if (ImGui::Begin(window.second.second->get_name().c_str()))
			{
				window.second.second->render();
				ImGui::End();
			}
		}
	}

	ImGui::End();
}

void WindowManager::on_created()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::Begin("DockSpace", nullptr, window_flags);
	m_dock_id = ImGui::GetID("##MainDocker");
	ImGui::PopStyleVar(2);


	ImGui::DockSpace(m_dock_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_::ImGuiDockNodeFlags_NoResize
		| ImGuiDockNodeFlags_PassthruCentralNode);



	ImGui::End();

	for (auto& window : m_registeredWindows)
	{
		window.second.second->on_created();
	}
}

void WindowManager::on_resize(int width,int height)
{
	m_size.set(GNF_UVec2(width, height));
}

bool WindowManager::need_render()
{
	return *m_windowMode.get() != WINDOW_MODE_MINIMIZED;
}