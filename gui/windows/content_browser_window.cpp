#include "content_browser_window.h"

#include <boost/iterator.hpp>
#include <stack>

#include "../../imgui/imgui.h"
#include "../../core/typedefs.h"
#include "../../core/string/string_types.h"
#include "../../core/io/io_utils.h"
#include "../../manager/resource_manager.h"
#include "../../graphic/editor_image.h"
#include "../../core/io/file_type.h"
#include "../../core/folders.h"
ContentBrowserWindow::~ContentBrowserWindow()
{
	ResourceManager::get_singleton()->destroy_resource(m_textIcon);
	ResourceManager::get_singleton()->destroy_resource(m_folderIcon);

}

void ContentBrowserWindow::render()
{
	using namespace boost::filesystem;
	static float padding = 15.f;
	static float btn_size = 64.f;
	float cell_size = padding + btn_size;
	int column_count = m_size.get()->x / cell_size;
	if (column_count < 1)
		column_count = 1;

	ImGui::PushStyleColor(ImGuiCol_Button, { 0,0,0,0.3 });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0,0,0,0.6 });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0,0,0,0.7});

	if (ImGui::Button("<<"))
	{
		if(m_path.has_parent_path() && m_path.parent_path().has_parent_path())
			m_path = m_path.parent_path();
	}

	

	auto iterPath = m_path;
	std::stack<std::string> strStack;
	while (iterPath.has_parent_path())
	{
		strStack.push(iterPath.filename().string());
		iterPath = iterPath.parent_path();
	}

	while (!strStack.empty())
	{
		ImGui::SameLine();
		ImGui::Button(strStack.top().c_str());
		strStack.pop();
	}

	ImGui::PopStyleColor(3);

	ImGui::Columns(column_count, 0, false);

	for (directory_iterator itr(m_path); itr != directory_iterator(); ++itr)
	{
		auto ex = file_name_to_file_type(itr->path());
		//X TODO : Needs to get from a global manager
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0.2f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0.3f));

		switch (ex)
		{
		case FILE_TYPE_UNKNOWN:
		case FILE_TYPE_TXT:
			ImGui::ImageButton(((EditorImage*)m_textIcon)->get_image(), { btn_size,btn_size });
			break;
		case FILE_TYPE_FOLDER:
			ImGui::ImageButton(((EditorImage*)m_folderIcon)->get_image(), { btn_size,btn_size });
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if(boost::filesystem::exists(itr->path()))
					m_path /= itr->path().filename();
			}
			break;
		}
		//ImGui::Button(itr->path().filename().string().c_str(),{btn_size,btn_size});
		ImGui::PopStyleColor(3);
		auto str = itr->path().filename().string();
		auto textWidth = ImGui::CalcTextSize(str.c_str()).x;

		auto textBegin = ImGui::GetCursorPosX() + ((btn_size - textWidth) * 0.5f);
		if(textBegin >= ImGui::GetCursorPosX())
			ImGui::SetCursorPosX(textBegin);

		ImGui::Text(str.c_str());
		ImGui::NextColumn();
	}
}

bool ContentBrowserWindow::on_created()
{
	m_textIcon = ResourceManager::get_singleton()->create_editor_image_resource("TEXT_ICON", &TEXT_ICON_PATH[0], this,
		(RESOURCE_USAGE)(RESOURCE_USAGE_SYSTEM | RESOURCE_USAGE_EDITOR));
	m_textIcon->load();

	//X TODO : NEED TO BE PARALLEL
	m_folderIcon = ResourceManager::get_singleton()->create_editor_image_resource("FOLDER_ICON", &FOLDER_ICON_PATH[0], this,
		(RESOURCE_USAGE)(RESOURCE_USAGE_SYSTEM | RESOURCE_USAGE_EDITOR));
	m_folderIcon->load();
	return m_textIcon->is_loaded() && m_folderIcon->is_loaded();
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
	return true;
}
