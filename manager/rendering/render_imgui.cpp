#include "render_imgui.h"


#include "../window_manager.h"

#include "render_device.h"

#include <vulkan/vulkan.h>

#include "../rendering/presentation_manager.h"

ImGuiDraw::ImGuiDraw()
{

}

void ImGuiDraw::init()
{



	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};
	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
	pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	auto err = vkCreateDescriptorPool(RenderDevice::get_singleton()->m_renderDevice.logicalDevice, &pool_info, nullptr, &imguiPool);


	ImGui_ImplVulkan_LoadFunctions([](const char* function_name, void* vulkan_instance) {
		return vkGetInstanceProcAddr(*(reinterpret_cast<VkInstance*>(vulkan_instance)), function_name);
		}, &RenderDevice::get_singleton()->m_instance.instance);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark();
	auto& style = ImGui::GetStyle();
	//!: Styles
	style.ScrollbarSize = 8.f;
	style.WindowRounding = 3.f;
	//!: Colors
	auto colors = style.Colors;
	colors[ImGuiCol_::ImGuiCol_WindowBg] =  ImVec4(38.f / 255.f, 43.f / 255.f, 58.f / 255.f, 1.f);
	colors[ImGuiCol_::ImGuiCol_TitleBg] = ImVec4(51.f / 255.f, 57.f / 255.f, 79.f / 255.f, 1.f);
	colors[ImGuiCol_::ImGuiCol_TitleBgActive] = ImVec4(51.f / 255.f, 57.f / 255.f, 79.f / 255.f, 1.f);
	colors[ImGuiCol_::ImGuiCol_MenuBarBg] = ImVec4(32.f / 255.f, 36.f / 255.f, 48.f / 255.f, 1.f);
	colors[ImGuiCol_::ImGuiCol_CheckMark] = ImVec4(1.f, 1.f, 1.f, 1.f);
	colors[ImGuiCol_::ImGuiCol_Border] = ImVec4(1.f, 1.f, 1.f, 0.f);

	/*colors[ImGuiCol_::ImGuiCol_FrameBg] = ImVec4(1.f, 1.f, 1.f, 0.f);
	colors[ImGuiCol_::ImGuiCol_FrameBgHovered] = ImVec4(1.f, 1.f, 1.f, 0.f);
	colors[ImGuiCol_::ImGuiCol_FrameBgActive] = ImVec4(1.f, 1.f, 1.f, 0.f);*/


	style.WindowBorderSize = 0;
	style.WindowRounding = 0;
	style.WindowMenuButtonPosition = ImGuiDir_::ImGuiDir_None;

	auto r = ImGui_ImplGlfw_InitForVulkan((WindowManager::get_singleton())->get_window(), true);

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = RenderDevice::get_singleton()->m_instance.instance;
	init_info.PhysicalDevice = RenderDevice::get_singleton()->m_renderDevice.physicalDev.physicalDev;
	init_info.Device = RenderDevice::get_singleton()->m_renderDevice.logicalDevice;
	init_info.QueueFamily = 0;
	init_info.Queue = RenderDevice::get_singleton()->m_renderDevice.mainQueue;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = imguiPool;
	init_info.Subpass = 0;
	init_info.MinImageCount = PresentationManager::get_singleton()->get_image_count() - 1;
	init_info.ImageCount = PresentationManager::get_singleton()->get_image_count();
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = VK_NULL_HANDLE;
	init_info.CheckVkResultFn = VK_NULL_HANDLE;

	ImGui_ImplVulkan_Init(&init_info, PresentationManager::get_singleton()->get_image_as_renderpass()->get_handle());
	RenderDevice::get_singleton()->reset_cmd_pool(0);

	auto cmd = RenderDevice::get_singleton()->m_renderDevice.pMainCommandBuffer;
	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	err = vkBeginCommandBuffer(cmd, &begin_info);
	ImGui_ImplVulkan_CreateFontsTexture(cmd);


	VkSubmitInfo inf = {};
	inf.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	inf.commandBufferCount = 1;
	inf.pCommandBuffers = &cmd;
	err = vkEndCommandBuffer(cmd);

	auto res = vkQueueSubmit(RenderDevice::get_singleton()->m_renderDevice.mainQueue, 1, &inf, nullptr);

	vkDeviceWaitIdle(RenderDevice::get_singleton()->m_renderDevice.logicalDevice);

	ImGui_ImplVulkan_DestroyFontUploadObjects();
	//vkDestroyDescriptorPool(device->m_device, imguiPool, nullptr);


	RenderDevice::get_singleton()->reset_cmd_pool(0);


	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

}
void ImGuiDraw::begin()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void ImGuiDraw::show_demo()
{
	ImGui::ShowDemoWindow();
}

void ImGuiDraw::end()
{
	// Rendering
	ImGui::Render();
}
void ImGuiDraw::fillCmd(VkCommandBuffer buff)
{
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), buff);
}
ImGuiDraw::~ImGuiDraw()
{
	//vkDestroyDescriptorPool(dev, imguiPool, nullptr);
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	vkDestroyDescriptorPool(RenderDevice::get_singleton()->m_renderDevice.logicalDevice, this->imguiPool, nullptr);
}