#include "content_browser_window.h"

#include <boost/iterator.hpp>

#include "../../imgui/imgui.h"
#include "../../core/typedefs.h"
#include "../../core/string/string_types.h"
#include "../../core/io/io_utils.h"

void ContentBrowserWindow::render()
{
	using namespace boost::filesystem;
	static float padding = 10.f;
	static float btn_size = 64.f;
	float cell_size = padding + btn_size;
	int column_count = m_size.get()->x / cell_size;
	if (column_count < 1)
		column_count = 1;
	ImGui::Columns(column_count, 0, false);

	for (directory_iterator itr(m_path); itr != directory_iterator(); ++itr)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 1, 0, 1));
		ImGui::Button(itr->path().filename().string().c_str(),{btn_size,btn_size});
		ImGui::PopStyleColor();
		

		ImGui::NextColumn();
	}
	ImGui::LabelText("Hello", "Hello");
}

bool ContentBrowserWindow::set_path(const String& path)
{
	if (!is_path_exist(path))
	{
		return false;
	}
	if (is_file(path))
	{
		return false;
	}
	
	m_path = boost::filesystem::path(path);
		
}
