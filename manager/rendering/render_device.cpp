#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif __linux__
#define VK_USE_PLATFORM_XLIB_KHR
#elif __APPLE__
#define VK_USE_PLATFORM_METAL_EXT
#endif
#define IMGUI_IMPL_VULKAN_NO_PROTOTYPES 

#include "../memory_manager.h"

#define DEBUG_LAYER_NAME "VK_LAYER_KHRONOS_validation"
#include "../thread_pool_manager.h"

#include <boost/format.hpp>
#include "render_device.h"

#include "../logger_manager.h"
#include "../window_manager.h"
#include "../../core/version.h"
#include "../../core/utils_vulkan.h"
#include "../../core/typedefs.h"
#include "render_imgui.h"

#include "../../imgui/imgui.h"
#include "../../imgui/imgui_impl_glfw.h"
#include "../../imgui/imgui_impl_vulkan.h"
#include "../../gui/windows/scene_window.h"
#include <cmath>
#include <boost/bind/bind.hpp>

#undef max

static VkBool32 VKAPI_CALL vk_debug_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	(void)pUserData; // Unused argument

	std::string error = std::string("[VULKAN] [MESSENGER] Debug report from ObjectType: ") + std::to_string(uint32_t(pCallbackData->pObjects->objectType)) + "\n\nMessage: " + pCallbackData->pMessage + "\n\n";

	LoggerManager::get_singleton()->log_cout(RenderDevice::get_singleton(), error.c_str(), Logger::DEBUG);
	return VK_FALSE;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugReportCallback
(
	VkDebugReportFlagsEXT      flags,
	VkDebugReportObjectTypeEXT objectType,
	uint64_t                   object,
	size_t                     location,
	int32_t                    messageCode,
	const char* pLayerPrefix,
	const char* pMessage,
	void* UserData
)
{
	// https://github.com/zeux/niagara/blob/master/src/device.cpp   [ignoring performance warnings]
	// This silences warnings like "For optimal performance image layout should be VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL instead of GENERAL."
	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
		return VK_FALSE;

	LoggerManager::get_singleton()->log_cout(RenderDevice::get_singleton(), boost::str(boost::format("[VULKAN] [REPORTER] [%1%] %2%") % pLayerPrefix % pMessage).c_str(),
		Logger::DEBUG);

	return VK_FALSE;
}

RenderDevice::RenderDevice() : m_quitting(this,false,"IsQuitting")
{
}

RenderDevice::~RenderDevice()
{
	if (m_instanceLoaded && m_instance.instance != nullptr)
	{
		m_quitting.set(true);
		MemoryManager::get_singleton()->destroy_object(imguiDraw);

		vkDestroyFence(m_renderDevice.logicalDevice, m_renderDevice.mainQueueFinishedFence, nullptr);
		vkDestroyFence(m_renderDevice.logicalDevice, m_renderDevice.presentQueueFinishedFence, nullptr);

		vkDestroySemaphore(m_renderDevice.logicalDevice, m_renderDevice.imageAcquiredSemaphore, nullptr);
		vkDestroySemaphore(m_renderDevice.logicalDevice, m_renderDevice.renderCompleteSemaphore, nullptr);
		vkDestroySemaphore(m_renderDevice.logicalDevice, m_renderDevice.renderSceneCompleteSemaphore, nullptr);

		for (int i = 0; i < m_swapchain.frameBuffers.size(); i++)
		{
			vkDestroyFramebuffer(m_renderDevice.logicalDevice, m_swapchain.frameBuffers[i], nullptr);
		}

		for (int i = 0; i < m_swapchain.swapchainImageViews.size(); i++)
		{
			vkDestroyImageView(m_renderDevice.logicalDevice, m_swapchain.swapchainImageViews[i], nullptr);
		}

		vkDestroyRenderPass(m_renderDevice.logicalDevice, m_swapchain.renderPass,nullptr);
		if (m_swapchain.swapchain != nullptr)
			vkDestroySwapchainKHR(m_renderDevice.logicalDevice, m_swapchain.swapchain, nullptr);

		if (m_renderDevice.presentCommandPool != nullptr)
			vkDestroyCommandPool(m_renderDevice.logicalDevice, m_renderDevice.presentCommandPool, nullptr);

		for (int i = 0; i < m_renderDevice.mainQueueCommandPools.size(); i++)
		{
			vkDestroyCommandPool(m_renderDevice.logicalDevice, m_renderDevice.mainQueueCommandPools[i], nullptr);

		}

		if (m_renderDevice.logicalDevice != nullptr)
		{
			vkDestroyDevice(m_renderDevice.logicalDevice, nullptr);
		}

		if (m_instance.messenger != nullptr)
		{
			auto pvkDestroyDebugUtilsMessengerEXT = PFN_vkDestroyDebugUtilsMessengerEXT(vkGetInstanceProcAddr(m_instance.instance, "vkDestroyDebugUtilsMessengerEXT"));
			if (pvkDestroyDebugUtilsMessengerEXT != nullptr)
				pvkDestroyDebugUtilsMessengerEXT(m_instance.instance, m_instance.messenger, nullptr);
		}
		if (m_instance.reportCallback != nullptr)
		{
			auto pvkDestroyDebugReportCallbackEXT = PFN_vkDestroyDebugReportCallbackEXT(vkGetInstanceProcAddr(m_instance.instance, "vkDestroyDebugReportCallbackEXT"));
			if (pvkDestroyDebugReportCallbackEXT != nullptr)
				pvkDestroyDebugReportCallbackEXT(m_instance.instance, m_instance.reportCallback, nullptr);

		}
		if (m_instance.surface != nullptr)
			vkDestroySurfaceKHR(m_instance.instance, m_instance.surface, nullptr);
		vkDestroyInstance(m_instance.instance, nullptr);
	}
}

bool RenderDevice::init()
{
	m_instance.format.format =	VK_FORMAT_B8G8R8A8_UNORM;
	m_instance.format.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

	bool succeeded = true;
	succeeded = init_vk_instance();
	if (!succeeded)
		return false;
	succeeded = init_vk_device();
	if (!succeeded)
		return false;
	succeeded = save_vk_physical_device();
	if (!succeeded)
		return false;
	succeeded = init_vk_logical_device();
	if (!succeeded)
		return false;
	expose_queues();
	succeeded = create_command_pools();
	if (!succeeded)
		return false;
	succeeded = init_renderpass();
	if (!succeeded)
		return false;
	succeeded = init_vk_swapchain();
	if (!succeeded)
		return false;
	succeeded = init_vk_syncs();
	if (!succeeded)
		return false;
	init_command_buffers();
	imguiDraw = MemoryManager::get_singleton()->new_object<ImGuiDraw>("ImGuiRenderer");
	imguiDraw->init();

	//m_renderScene = MemoryManager::get_singleton()->new_object<RenderScene>("SceneRenderer", m_renderDevice.logicalDevice,m_renderDevice.physicalDev.physicalDev);

	init_subs();
	return true;
}

//LoggerServer::get_singleton()->log_cout(this,"Couldn't initalize debug mode",Logger::WARNING);
void RenderDevice::on_size_changed(const GNF_UVec2& size)
{
	m_instance.surfaceExtent.height = size.y;
	m_instance.surfaceExtent.width = size.x;
	swapchain_needs_validate = true;
	//init_vk_swapchain();


}

void RenderDevice::init_subs()
{
	windowResizeConnection = WindowManager::get_singleton()->get_size()->subscribe_changed_event(boost::bind(&RenderDevice::on_size_changed, this, boost::placeholders::_1));
}

bool RenderDevice::init_vk_instance()
{
	{
		// Application Info
		VkApplicationInfo applicationInfo;
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pNext = nullptr;
		applicationInfo.pApplicationName = std::string(_STR_XDEF(GNF_APP_NAME_FULL)).c_str();
		applicationInfo.pEngineName = "NO ENGINE";
		applicationInfo.engineVersion = GNF_VERSION_AS_HEX;
		applicationInfo.apiVersion = VK_API_VERSION_1_2;


		// First get all instance layer

		std::vector<VkLayerProperties> allInstanceLayers;

		if (!get_all_instance_layers(allInstanceLayers))
		{
			return false;
		}

		bool isDebugEnabled = true;
	
		bool isDebugLayerAdded = false;
		if (isDebugEnabled)
		{
			int index;
			isDebugLayerAdded = find_layer_by_name(&allInstanceLayers, DEBUG_LAYER_NAME, index);
			if (!isDebugLayerAdded)
				LoggerManager::get_singleton()->log_cout(this, boost::str(boost::format("Debug Layer was enabled but there was no layer with named %1%") % DEBUG_LAYER_NAME).c_str(), Logger::WARNING);
			else
			{
				m_instance.enabledProps.enabledLayers.push_back(allInstanceLayers[index]);
				LoggerManager::get_singleton()->log_cout(this, boost::str(boost::format("Debug Layer added with named %1%") % DEBUG_LAYER_NAME).c_str(), Logger::INFO);
			}
		}

		//X TODO : Plugin Here


		//X Check the array if plugins added same thing more than once
		m_instance.enabledProps.enabledLayers.erase(std::unique(m_instance.enabledProps.enabledLayers.begin(), m_instance.enabledProps.enabledLayers.end(),
			[](const VkLayerProperties& lhs, const VkLayerProperties& rhs) {
				return std::strcmp(lhs.layerName, rhs.layerName) == 0;
			}),
			m_instance.enabledProps.enabledLayers.end());


		std::vector<const char*> enabledLayers(m_instance.enabledProps.enabledLayers.size());

		// Copy to enabledLayers

		for (int i = 0; i < m_instance.enabledProps.enabledLayers.size(); i++)
		{
			enabledLayers[i] = m_instance.enabledProps.enabledLayers[i].layerName;
		}

		auto all_exs = get_all_instance_exs_by_layers(&enabledLayers);

		std::vector<VkExtensionProperties> implicitExs;

		get_instance_implicit_exs(implicitExs);
		all_exs.emplace(IMPLICIT_EXTENSIONS_NAME, std::vector<VkExtensionProperties>());
		for (const auto& ex : implicitExs)
		{
			all_exs[IMPLICIT_EXTENSIONS_NAME].push_back(ex);
		}


		// NOW WE GOT ALL EXTENSIONS

		// Be sure that extensions of debug layer added
		if (isDebugEnabled && isDebugLayerAdded)
		{
			m_instance.enabledProps.enabledExtensions.emplace(std::string(DEBUG_LAYER_NAME), all_exs[DEBUG_LAYER_NAME]);
		}

		// Be sure for system surface extensions added
		// This is for surface creation
#ifdef VK_USE_PLATFORM_WIN32_KHR 
		std::vector<const char*> names = {
		 VK_KHR_SURFACE_EXTENSION_NAME ,VK_KHR_WIN32_SURFACE_EXTENSION_NAME
		};
		std::vector<int> indexes;
		bool allFounded = find_instance_exs_by_names(&all_exs[IMPLICIT_EXTENSIONS_NAME], &names, indexes);

		if (!allFounded)
			return false;
		m_instance.enabledProps.enabledExtensions[IMPLICIT_EXTENSIONS_NAME].push_back(all_exs[IMPLICIT_EXTENSIONS_NAME][indexes[0]]);
		m_instance.enabledProps.enabledExtensions[IMPLICIT_EXTENSIONS_NAME].push_back(all_exs[IMPLICIT_EXTENSIONS_NAME][indexes[1]]);

#elif defined VK_USE_PLATFORM_XLIB_KHR 
		std::vector<const char*> names = {
	 VK_KHR_SURFACE_EXTENSION_NAME ,VK_KHR_XLIB_SURFACE_EXTENSION_NAME
		};
		std::vector<int> indexes;
		bool allFounded = find_instance_exs_by_names(&all_exs[IMPLICIT_EXTENSIONS_NAME], &names, indexes);

		if (!allFounded)
			return false;
		m_instance.enabledProps.enabledExtensions[IMPLICIT_EXTENSIONS_NAME].push_back(all_exs[IMPLICIT_EXTENSIONS_NAME][indexes[0]]);
		m_instance.enabledProps.enabledExtensions[IMPLICIT_EXTENSIONS_NAME].push_back(all_exs[IMPLICIT_EXTENSIONS_NAME][indexes[1]]);

#elif defined VK_USE_PLATFORM_METAL_EXT 
		std::vector<const char*> names = {
	 VK_KHR_SURFACE_EXTENSION_NAME ,"VK_EXT_metal_surface","VK_KHR_portability_enumeration"
		};
		std::vector<int> indexes;
		bool allFounded = find_instance_exs_by_names(&all_exs[IMPLICIT_EXTENSIONS_NAME], &names, indexes);

		if (!allFounded)
			return false;
		m_instance.enabledProps.enabledExtensions[IMPLICIT_EXTENSIONS_NAME].push_back(all_exs[IMPLICIT_EXTENSIONS_NAME][indexes[0]]);
		m_instance.enabledProps.enabledExtensions[IMPLICIT_EXTENSIONS_NAME].push_back(all_exs[IMPLICIT_EXTENSIONS_NAME][indexes[1]]);
		m_instance.enabledProps.enabledExtensions[IMPLICIT_EXTENSIONS_NAME].push_back(all_exs[IMPLICIT_EXTENSIONS_NAME][indexes[2]]);
#else
		DONT COMPILE
#endif 


			//X TODO : PLUGIN GOES HERE ...
			//X Check the array if plugins added same thing more than once
			for (auto& exs : m_instance.enabledProps.enabledExtensions)
			{
				exs.second.erase(std::unique(exs.second.begin(), exs.second.end(),
					[](const VkExtensionProperties& lhs, const VkExtensionProperties& rhs) {
						return std::strcmp(lhs.extensionName, rhs.extensionName) == 0;
					}),
					exs.second.end());
			}

		//X MAKE READY EXTENSIONS FOR VULKAN

		uint32_t vsize = 0;
		for (auto& exs : m_instance.enabledProps.enabledExtensions)
		{
			vsize += (uint32_t)exs.second.size();
		}

		std::vector<const char*> enabledExs(vsize);

		int vindex = 0;
		// Copy data to vector
		for (auto& exs : m_instance.enabledProps.enabledExtensions)
		{
			for (int i = 0; i < exs.second.size(); i++)
			{
				enabledExs[vindex] = exs.second[i].extensionName;
				vindex++;
			}
		}

		//X NOW WE ARE READY TO CREATE INSTANCE
		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pNext = nullptr;

		createInfo.pApplicationInfo = &applicationInfo;
		createInfo.enabledLayerCount = (uint32_t)enabledLayers.size();
		createInfo.ppEnabledLayerNames = enabledLayers.data();
		createInfo.enabledExtensionCount = (uint32_t)enabledExs.size();
		createInfo.ppEnabledExtensionNames = enabledExs.data();
#if defined VK_USE_PLATFORM_METAL_EXT 
		createInfo.flags = 1;
#else
		createInfo.flags = 0;
#endif
		auto res = vkCreateInstance(&createInfo, nullptr, &m_instance.instance);
		if (res != VK_SUCCESS)
			return false;

		//X Init Function pointers of instance
		m_instanceLoaded = true;

		if (isDebugEnabled)
		{
			create_debug_messenger(m_instance.instance, &m_instance.messenger, &m_instance.reportCallback,
				vk_debug_messenger_callback, VulkanDebugReportCallback);
		}

		m_isDebugEnabled = isDebugEnabled;
	}

	//X Create Surface
	{
		if (glfwCreateWindowSurface(m_instance.instance, WindowManager::get_singleton()->get_window(), nullptr, &m_instance.surface) != VK_SUCCESS)
			return false;
	}

	return true;
}

void RenderDevice::ready_ui_data()
{

	WindowManager::get_singleton()->render();
	imguiDraw->end();
}

bool RenderDevice::init_command_buffers()
{

	for (int i = 1; i < m_renderDevice.mainQueueCommandPools.size(); i++)
	{
		VkCommandBufferAllocateInfo inf = {};
		inf.commandBufferCount = 2;
		inf.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		inf.pNext = nullptr;
		inf.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		inf.commandPool = m_renderDevice.mainQueueCommandPools[i];
		m_renderDevice.commandBuffers.emplace(i, std::vector<VkCommandBuffer>());
		m_renderDevice.commandBuffers[i].resize(2);

		vkAllocateCommandBuffers(m_renderDevice.logicalDevice, &inf, m_renderDevice.commandBuffers[i].data());
	}

	m_renderDevice.commandBuffers.emplace(0, std::vector<VkCommandBuffer>());
	m_renderDevice.commandBuffers[0].resize(2);

	VkCommandBufferAllocateInfo inf = {};
	inf.commandBufferCount = 1;
	inf.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	inf.pNext = nullptr;
	inf.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	inf.commandPool = m_renderDevice.mainQueueCommandPools[0];
	vkAllocateCommandBuffers(m_renderDevice.logicalDevice, &inf, &(m_renderDevice.commandBuffers[0][0]));

	m_renderDevice.pMainCommandBuffer = m_renderDevice.commandBuffers[0][0];

	vkAllocateCommandBuffers(m_renderDevice.logicalDevice, &inf, &(m_renderDevice.commandBuffers[0][1]));

	m_renderDevice.pSceneCommandBuffer = m_renderDevice.commandBuffers[0][1];

	return true;
}

void RenderDevice::reset_things()
{
	vkWaitForFences(m_renderDevice.logicalDevice, 1, &m_renderDevice.mainQueueFinishedFence, VK_TRUE, UINT64_MAX);

	for (int i = 0; i < m_renderDevice.mainQueueCommandPools.size(); i++)
	{
		vkResetCommandPool(m_renderDevice.logicalDevice, m_renderDevice.mainQueueCommandPools[i], 0);
	}
	//vkResetCommandPool(m_renderDevice.logicalDevice, m_renderDevice.mainQueueCommandPools[0], 0);
	vkResetFences(m_renderDevice.logicalDevice, 1, &m_renderDevice.mainQueueFinishedFence);
}

void RenderDevice::set_next_image()
{
	auto err = vkAcquireNextImageKHR(m_renderDevice.logicalDevice, m_swapchain.swapchain, UINT64_MAX, m_renderDevice.imageAcquiredSemaphore, nullptr, &currentImageIndex);
	//LoggerServer::get_singleton()->log_cout(this, "Next Image has came out", Logger::LOG_LEVEL::DEBUG);

	if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
	{
		//swapchain rebuild
		swapchain_needs_validate = true;
		LoggerManager::get_singleton()->log_cout(this, "IMAGE IS BUGGY", Logger::LOG_LEVEL::ERROR);
	}

	m_swapchain.currentFrameBuffer = m_swapchain.frameBuffers[currentImageIndex];
}

void RenderDevice::beginFrame()
{
	/*for (int i = 0; i < m_renderDevice.mainQueueCommandPools.size(); i++)
	{
		vkResetCommandPool(m_renderDevice.logicalDevice, m_renderDevice.mainQueueCommandPools[i], 0);
	}*/

	//LoggerServer::get_singleton()->log_cout(this, "Wait next image", Logger::LOG_LEVEL::DEBUG);

	//auto err = vkAcquireNextImageKHR(m_renderDevice.logicalDevice, m_swapchain.swapchain, UINT64_MAX, m_renderDevice.imageAcquiredSemaphore, nullptr, &currentImageIndex);
	////LoggerServer::get_singleton()->log_cout(this, "Next Image has came out", Logger::LOG_LEVEL::DEBUG);

	//if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
	//{
	//	//swapchain rebuild
	//	LoggerServer::get_singleton()->log_cout(this, "IMAGE IS BUGGY", Logger::LOG_LEVEL::ERROR);
	//}

	//m_swapchain.currentFrameBuffer = m_swapchain.frameBuffers[currentImageIndex];

	////LoggerServer::get_singleton()->log_cout(this, "Wait fence", Logger::LOG_LEVEL::DEBUG);

	//err = vkWaitForFences(m_renderDevice.logicalDevice, 1, &m_renderDevice.mainQueueFinishedFence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking
	////if (err != VK_SUCCESS)
	////	return;

	////LoggerServer::get_singleton()->log_cout(this, "Reset fence", Logger::LOG_LEVEL::DEBUG);

	//err = vkResetFences(m_renderDevice.logicalDevice, 1, &m_renderDevice.mainQueueFinishedFence);
	////if (err != VK_SUCCESS)
	////{
	////	LoggerServer::get_singleton()->log_cout(this, "Error Occurred while reseting fence", Logger::LOG_LEVEL::ERROR);
	////	LoggerServer::get_singleton()->log_cout(this, std::to_string(err), Logger::LOG_LEVEL::ERROR);

	////}

	//LoggerServer::get_singleton()->log_cout(this, "Fences resetted try to begin new frame", Logger::LOG_LEVEL::DEBUG);

	ImGui_ImplVulkan_NewFrame();
	ImGui::NewFrame();

}

void RenderDevice::on_created()
{
	//X TODO : Calculate to size of the Window that will present the RenderDevice
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplVulkan_NewFrame();
	ImGui::NewFrame();

	WindowManager::get_singleton()->on_created();

	ImGui::EndFrame();

	// In here give it to RenderDevice
	// For now it is same size with swapchain
	GNF_UVec2 size;
	size.x = m_instance.surfaceExtent.width;
	size.y = m_instance.surfaceExtent.height;

	vkResetCommandPool(m_renderDevice.logicalDevice, m_renderDevice.mainQueueCommandPools[0], 0);
}

void RenderDevice::pre_render()
{
	
	WindowManager::get_singleton()->pre_render();

	ImGui::EndFrame();
}

bool RenderDevice::render_things(tf::Subflow& subflow)
{

	auto updt = subflow.emplace([n = this, imguiDraw = imguiDraw](tf::Subflow& subflow) {
		n->render_ui(subflow);
		});

	auto endUpdt = subflow.emplace([n = this, imguiDraw = imguiDraw]() {

		imguiDraw->end();

		});

	auto fillPhase = subflow.emplace([n = this]() {

		auto cmd = n->get_main_cmd();

		vkBeginCommandBuffer(cmd, n->get_main_begin_inf());


		vkCmdBeginRenderPass(cmd, n->get_main_renderpass_begin_inf(), VK_SUBPASS_CONTENTS_INLINE);

		n->fillCmd(cmd);
		// After filling cmd start to execute
		vkCmdEndRenderPass(cmd);
		vkEndCommandBuffer(cmd);

		vkQueueSubmit(n->get_render_device().mainQueue, 1, n->get_main_submit_info(1, &cmd), n->get_render_device().mainQueueFinishedFence);

		});

#ifdef _DEBUG
	updt.name("Render UI");
	endUpdt.name("FinishCollectingImGuiData");
	fillPhase.name("Fill Buffer");
#endif
	updt.precede(endUpdt);
	endUpdt.precede(fillPhase);

	return m_canContinue;
}


void RenderDevice::beginFrameW()
{
	ImGui_ImplGlfw_NewFrame();
}

void RenderDevice::fill_and_execute_cmd()
{
	auto sss = vkBeginCommandBuffer(m_renderDevice.pMainCommandBuffer, this->get_main_begin_inf());

	auto inf = this->get_main_renderpass_begin_inf();
	vkCmdBeginRenderPass(m_renderDevice.pMainCommandBuffer, inf, VK_SUBPASS_CONTENTS_INLINE);

	imguiDraw->fillCmd(m_renderDevice.pMainCommandBuffer);

	vkCmdEndRenderPass(m_renderDevice.pMainCommandBuffer);
	vkEndCommandBuffer(m_renderDevice.pMainCommandBuffer);

	auto t = this->get_main_submit_info(1, &m_renderDevice.pMainCommandBuffer);
	vkQueueSubmit(m_renderDevice.mainQueue, 1, t, m_renderDevice.mainQueueFinishedFence);
}

void RenderDevice::render_scene()
{
	m_renderScene = WindowManager::get_singleton()->need_render("Scene");
	if (m_renderScene)
	{
		((SceneWindow*)WindowManager::get_singleton()->get_registered_window("Scene"))->get_render_scene()->fill_cmd(m_renderDevice.pSceneCommandBuffer);
		VkSubmitInfo inf = {};
		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		inf.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		inf.pNext = nullptr;
		inf.commandBufferCount = 1;
		inf.pCommandBuffers = &m_renderDevice.pSceneCommandBuffer;
		inf.signalSemaphoreCount = 1;
		inf.pSignalSemaphores = &m_renderDevice.renderSceneCompleteSemaphore;
		inf.pWaitDstStageMask = &waitStage;
		vkQueueSubmit(m_renderDevice.mainQueue, 1, &inf, nullptr);
	}
}

void RenderDevice::render2()
{

	auto err = vkAcquireNextImageKHR(m_renderDevice.logicalDevice, m_swapchain.swapchain, UINT64_MAX, m_renderDevice.imageAcquiredSemaphore, nullptr, &currentImageIndex);
	//LoggerServer::get_singleton()->log_cout(this, "Next Image has came out", Logger::LOG_LEVEL::DEBUG);

	if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
	{
		//swapchain rebuild
		LoggerManager::get_singleton()->log_cout(this, "IMAGE IS BUGGY", Logger::LOG_LEVEL::ERROR);
	}

	m_swapchain.currentFrameBuffer = m_swapchain.frameBuffers[currentImageIndex];


	vkWaitForFences(m_renderDevice.logicalDevice, 1, &m_renderDevice.mainQueueFinishedFence, VK_TRUE, UINT64_MAX);

	vkResetCommandPool(m_renderDevice.logicalDevice, m_renderDevice.mainQueueCommandPools[0], 0);
	vkResetFences(m_renderDevice.logicalDevice, 1, &m_renderDevice.mainQueueFinishedFence);

	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplVulkan_NewFrame();
	ImGui::NewFrame();

	ImGui::ShowDemoWindow();

	ImGui::Render();




	auto sss = vkBeginCommandBuffer(m_renderDevice.pMainCommandBuffer, this->get_main_begin_inf());

	auto inf = this->get_main_renderpass_begin_inf();
	vkCmdBeginRenderPass(m_renderDevice.pMainCommandBuffer, inf, VK_SUBPASS_CONTENTS_INLINE);

	imguiDraw->fillCmd(m_renderDevice.pMainCommandBuffer);

	vkCmdEndRenderPass(m_renderDevice.pMainCommandBuffer);
	vkEndCommandBuffer(m_renderDevice.pMainCommandBuffer);


	vkQueueSubmit(m_renderDevice.mainQueue, 1, this->get_main_submit_info(1, &m_renderDevice.pMainCommandBuffer), m_renderDevice.mainQueueFinishedFence);

	swapbuffers();
}

void RenderDevice::swapbuffers()
{

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;

	presentInfo.pSwapchains = &m_swapchain.swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &m_renderDevice.renderCompleteSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &currentImageIndex;

	auto sadasdas = vkQueuePresentKHR(m_renderDevice.mainQueue, &presentInfo);
}

void RenderDevice::handleError()
{
	m_canContinue = false;
}

void RenderDevice::fillCmd(VkCommandBuffer buff)
{
	imguiDraw->fillCmd(buff);
}
void RenderDevice::render_ui(tf::Subflow& subflow)
{
	auto task = subflow.emplace([imguiDraw = imguiDraw]() {imguiDraw->show_demo(); });

#ifdef _DEBUG
	task.name("ImGui Draw Fill");
#endif
}

bool RenderDevice::init_vk_device()
{
	std::vector<VkPhysicalDevice> devs;
	if (!get_all_physical_devices(m_instance.instance, devs))
		return false;

	std::vector<std::string> requiredExtensionsForPhysicalDevice = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	std::vector<std::vector<VkExtensionProperties>> requiredExtensionPropsForPhysicalDevices;

	auto dev = devs.begin();
	while (dev != devs.end())
	{
		std::vector<VkExtensionProperties> requiredExtensionPropsForPhysicalDevice;

		if (check_device_extension_support(*dev.operator->(), &requiredExtensionsForPhysicalDevice, requiredExtensionPropsForPhysicalDevice))
		{
			dev++;
		}
		else
		{
			dev = devs.erase(dev);
		}
	}

	if (devs.size() == 0)
		return false;


	// Suface is supported now check its capabilities
	dev = devs.begin();

	while (dev != devs.end())
	{
		SwapChainSupportDetails details;
		bool getDetails = get_swap_chain_support_details(*dev.operator->(), m_instance.surface, details);

		if (!getDetails)
		{
			dev = devs.erase(dev);
			continue;
		}
		bool doesSupportPresent = false;
		for (const auto& presentMode : details.presentModes)
		{
			if (presentMode == m_instance.presentMode)
			{
				doesSupportPresent = true;
			}
		}

		bool doesSupportTripleBuff = true;
		auto min_number_of_images = details.capabilities.minImageCount + 1;
		if ((details.capabilities.maxImageCount > 0) &&
			(min_number_of_images > details.capabilities.maxImageCount)) {
			doesSupportTripleBuff = false;
		}

		bool doesSupportFormat = false;
		for (const auto& availableFormat : details.formats) {
			if (availableFormat.format == m_instance.format.format && availableFormat.colorSpace == m_instance.format.colorSpace) {
				doesSupportFormat = true;
			}
		}


		bool doesSupportAllReq = doesSupportPresent && doesSupportTripleBuff && doesSupportFormat;

		if (doesSupportAllReq)
		{
			dev++;
		}
		else
		{
			dev = devs.erase(dev);
		}
	}

	// Surface capabilities is supported. Now check Queue Futures

	auto queueRequirements = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
	dev = devs.begin();
	std::vector<int> queueIndices;
	while (dev != devs.end())
	{
		int index = -1;
		if (check_queue_support(*dev.operator->(), queueRequirements, index))
		{
			/*	VkBool32 supportSurface;
				vkGetPhysicalDeviceSurfaceSupportKHR(*dev.operator->(), index, m_instance.surface, &supportSurface)*/;

				queueIndices.push_back(index);
				dev++;

		}
		else
		{
			dev = devs.erase(dev);
		}
	}

	// Check config file 

	VkPhysicalDeviceType physicalType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

	if (devs.size() == 0)
		return false;

	bool physicalDeviceSelected = false;

	for (int i = 0; i < devs.size(); i++)
	{
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(devs[i], &props);
		if (props.deviceType == physicalType)
		{
			physicalDeviceSelected = true;
			m_renderDevice.physicalDev.physicalDev = devs[i];
			m_renderDevice.mainQueueFamilyIndex = queueIndices[i];
		}
	}

	if (physicalDeviceSelected == false)
	{
		physicalDeviceSelected = true;
		m_renderDevice.physicalDev.physicalDev = devs[0];
		m_renderDevice.mainQueueFamilyIndex = queueIndices[0];

	}
	return physicalDeviceSelected;
}


bool RenderDevice::save_vk_physical_device()
{
	bool getted = get_swap_chain_support_details(m_renderDevice.physicalDev.physicalDev, m_instance.surface, m_swapChainDetails);
	if (!getted)
		return false;
	m_instance.surfaceImageCount = m_swapChainDetails.capabilities.minImageCount + 1;

	// Extent of scrren
	if (m_swapChainDetails.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		m_instance.surfaceExtent = m_swapChainDetails.capabilities.currentExtent;
	}
	else {
		int wndSizeX = WindowManager::get_singleton()->get_size_r()->x;
		int wndSizeY = WindowManager::get_singleton()->get_size_r()->y;
		VkExtent2D actualExtent = {
			static_cast<uint32_t>(wndSizeX),
			static_cast<uint32_t>(wndSizeY)
		};

		actualExtent.width = std::clamp(actualExtent.width, m_swapChainDetails.capabilities.minImageExtent.width, m_swapChainDetails.capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, m_swapChainDetails.capabilities.minImageExtent.height, m_swapChainDetails.capabilities.maxImageExtent.height);

		m_instance.surfaceExtent = actualExtent;
	}

	vkGetPhysicalDeviceFeatures(m_renderDevice.physicalDev.physicalDev, &(m_renderDevice.physicalDev.physicalDevFeatures));

	vkGetPhysicalDeviceProperties(m_renderDevice.physicalDev.physicalDev, &(m_renderDevice.physicalDev.physicalDevProperties));

	auto queues = get_all_queue_families_by_device(m_renderDevice.physicalDev.physicalDev);

	bool found = false;
	for (unsigned int i = 0; i < queues.size(); i++)
	{
		VkBool32 supportSurface;
		vkGetPhysicalDeviceSurfaceSupportKHR(m_renderDevice.physicalDev.physicalDev, i, m_instance.surface, &supportSurface);
		if (supportSurface == VK_TRUE)
		{
			if (i == m_renderDevice.mainQueueFamilyIndex)
			{
				found = true;
				break;
			}
		}
	}

	return found;
}


bool RenderDevice::init_vk_logical_device()
{
	//X TODO : MAYBE CAN SELECT MORE THAN ONE QUEUE FAMILY OR SAME FAMILY BUT MORE THAN ONE QUEUE
	{
		QueueCreateInf queueCreateInf;


		std::vector<float> mainPriority = { 1.f };
		queueCreateInf.add_create_info(m_renderDevice.mainQueueFamilyIndex, mainPriority);

		m_renderDevice.mainQueueIndex = 0;

		std::vector<VkLayerProperties> allLayerProps;
		if (!get_all_device_layers(m_renderDevice.physicalDev.physicalDev, allLayerProps))
			return false;

		// ADD LAYER AND EXTENSION 
		if (m_isDebugEnabled && m_instance.enabledProps.enabledExtensions.find(DEBUG_LAYER_NAME) != m_instance.enabledProps.enabledExtensions.end())
		{
			for (const auto& prop : m_instance.enabledProps.enabledLayers)
			{
				if (strcmp(prop.layerName, DEBUG_LAYER_NAME) == 0)
				{
					m_renderDevice.enabledProps.enabledLayers.push_back(prop);
				}
			}
		}

		m_renderDevice.enabledProps.enabledExtensions.emplace(IMPLICIT_EXTENSIONS_NAME, std::vector<VkExtensionProperties>());

		std::vector<VkExtensionProperties> exPropsI;
		get_device_implicit_exs(m_renderDevice.physicalDev.physicalDev, exPropsI);

		int exIndex = -1;
		for (int i = 0; i < exPropsI.size(); i++)
		{
			if (strcmp(exPropsI[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
			{
				exIndex = i;
				break;
			}
		}
		if (exIndex != -1)
			m_renderDevice.enabledProps.enabledExtensions[IMPLICIT_EXTENSIONS_NAME].push_back(exPropsI[exIndex]);


		//X TODO : PLUGIN GOES HERE

		//X Check the array if plugins added same thing more than once
		m_renderDevice.enabledProps.enabledLayers.erase(std::unique(m_renderDevice.enabledProps.enabledLayers.begin(), m_renderDevice.enabledProps.enabledLayers.end(),
			[](const VkLayerProperties& lhs, const VkLayerProperties& rhs) {
				return std::strcmp(lhs.layerName, rhs.layerName) == 0;
			}),
			m_renderDevice.enabledProps.enabledLayers.end());


		//X Check the array if plugins added same thing more than once
		for (auto& exs : m_renderDevice.enabledProps.enabledExtensions)
		{
			exs.second.erase(std::unique(exs.second.begin(), exs.second.end(),
				[](const VkExtensionProperties& lhs, const VkExtensionProperties& rhs) {
					return std::strcmp(lhs.extensionName, rhs.extensionName) == 0;
				}),
				exs.second.end());
		}

		std::vector<const char*> enabledLayers(m_renderDevice.enabledProps.enabledLayers.size());

		int vexSize = 0;
		for (const auto& exs : m_renderDevice.enabledProps.enabledExtensions)
		{
			for (const auto& ex : exs.second)
			{
				vexSize++;
			}
		}

		std::vector<const char*> enabledExtensions(vexSize);

		// Make ready data for vulkan
		for (int i = 0; i < m_renderDevice.enabledProps.enabledLayers.size(); i++)
		{
			enabledLayers[i] = m_renderDevice.enabledProps.enabledLayers[i].layerName;
		}

		int vexIndex = 0;
		for (const auto& exs : m_renderDevice.enabledProps.enabledExtensions)
		{
			for (const auto& ex : exs.second)
			{
				enabledExtensions[vexIndex] = ex.extensionName;
				vexIndex++;
			}
		}


		// NOW CREATE LOGICAL DEVICE

		VkDeviceCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.queueCreateInfoCount = (uint32_t)queueCreateInf.get_queue_create_inf_count();
		createInfo.pQueueCreateInfos = queueCreateInf.data();
		createInfo.enabledLayerCount = (uint32_t)enabledLayers.size();
		createInfo.ppEnabledLayerNames = enabledLayers.data();
		createInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
		createInfo.ppEnabledExtensionNames = enabledExtensions.data();
		createInfo.pEnabledFeatures = &(m_renderDevice.physicalDev.physicalDevFeatures);

		if (auto res = vkCreateDevice(m_renderDevice.physicalDev.physicalDev, &createInfo, nullptr, &(m_renderDevice.logicalDevice)); res != VK_SUCCESS)
			return false;

	}
	return true;
}



void RenderDevice::expose_queues()
{
	// For now we are creating just one queue for main queue family
	vkGetDeviceQueue(m_renderDevice.logicalDevice, m_renderDevice.mainQueueFamilyIndex, m_renderDevice.mainQueueIndex, &(m_renderDevice.mainQueue));

}

bool RenderDevice::create_command_pools()
{
	auto poolCount = ThreadPoolManager::get_singleton()->get_thread_count();
	m_renderDevice.mainQueueCommandPools.resize(poolCount);
	for (int i = 0; i < poolCount; i++)
	{
		VkCommandPoolCreateInfo commandPoolInfo{};
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.pNext = nullptr;
		commandPoolInfo.flags = 0;
		commandPoolInfo.queueFamilyIndex = m_renderDevice.mainQueueFamilyIndex;

		auto res = vkCreateCommandPool(m_renderDevice.logicalDevice, &commandPoolInfo, nullptr, &(m_renderDevice.mainQueueCommandPools[i]));
		if (res != VK_SUCCESS)
			return false;
	}

	return true;
}

bool RenderDevice::init_vk_swapchain()
{
	VkSwapchainCreateInfoKHR swapchain_create_info = {
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		nullptr,
		0,
		m_instance.surface,
		(uint32_t)m_instance.surfaceImageCount,
		m_instance.format.format,
		m_instance.format.colorSpace,
		m_instance.surfaceExtent,
		1,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT , // Image Usage
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		nullptr,
		m_swapChainDetails.capabilities.currentTransform,
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		m_instance.presentMode,
		VK_TRUE,
		m_swapchain.swapchain
	};

	if (auto sres = vkCreateSwapchainKHR(m_renderDevice.logicalDevice, &swapchain_create_info, nullptr, &m_swapchain.swapchain); sres != VK_SUCCESS)
		return false;

	uint32_t imageCount = 0;
	if (vkGetSwapchainImagesKHR(m_renderDevice.logicalDevice, m_swapchain.swapchain, &imageCount, nullptr) != VK_SUCCESS)
	{
		return false;
	}

	m_swapchain.swapchainImages.resize(imageCount);
	if (vkGetSwapchainImagesKHR(m_renderDevice.logicalDevice, m_swapchain.swapchain, &imageCount, m_swapchain.swapchainImages.data()) != VK_SUCCESS)
	{
		return false;
	}

	// Create Image View

	m_swapchain.swapchainImageViews.resize(imageCount);

	VkImageViewCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = m_instance.format.format;

	// Default Usage 
	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;
	for (uint32_t i = 0; i < imageCount; i++)
	{

		createInfo.image = m_swapchain.swapchainImages[i];

		vkCreateImageView(m_renderDevice.logicalDevice, &createInfo, nullptr, &(m_swapchain.swapchainImageViews[i]));

	}


	// Create FrameBuffers

	m_swapchain.frameBuffers.resize(imageCount);

	//create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
	VkFramebufferCreateInfo fb_info = {};
	fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fb_info.pNext = nullptr;

	fb_info.renderPass = m_swapchain.renderPass;
	fb_info.attachmentCount = 1;
	fb_info.width = m_instance.surfaceExtent.width;
	fb_info.height = m_instance.surfaceExtent.height;
	fb_info.layers = 1;


	for (uint32_t i = 0; i < imageCount; i++)
	{
		fb_info.pAttachments = &(m_swapchain.swapchainImageViews[i]);
		if (vkCreateFramebuffer(m_renderDevice.logicalDevice, &fb_info, nullptr, &m_swapchain.frameBuffers[i]) != VK_SUCCESS)
		{
			return false;
		}
	}
	return true;
}

bool RenderDevice::validate_swapchain()
{
	VkSwapchainCreateInfoKHR swapchain_create_info = {
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		nullptr,
		0,
		m_instance.surface,
		(uint32_t)m_instance.surfaceImageCount,
		m_instance.format.format,
		m_instance.format.colorSpace,
		m_instance.surfaceExtent,
		1,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT , // Image Usage
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		nullptr,
		m_swapChainDetails.capabilities.currentTransform,
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		m_instance.presentMode,
		VK_TRUE,
		m_swapchain.swapchain
	};

	get_swap_chain_support_details(m_renderDevice.physicalDev.physicalDev, m_instance.surface, m_swapChainDetails);

	m_instance.surfaceExtent = m_swapChainDetails.capabilities.currentExtent;
	if (auto sres = vkCreateSwapchainKHR(m_renderDevice.logicalDevice, &swapchain_create_info, nullptr, &m_swapchain.swapchain); sres != VK_SUCCESS)
		return false;

	// Create Image Views
	uint32_t imageCount;
	vkGetSwapchainImagesKHR(m_renderDevice.logicalDevice, m_swapchain.swapchain, &imageCount, m_swapchain.swapchainImages.data());


	VkImageViewCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	info.format = m_instance.format.format;
	info.components.r = VK_COMPONENT_SWIZZLE_R;
	info.components.g = VK_COMPONENT_SWIZZLE_G;
	info.components.b = VK_COMPONENT_SWIZZLE_B;
	info.components.a = VK_COMPONENT_SWIZZLE_A;
	VkImageSubresourceRange image_range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	info.subresourceRange = image_range;
	for (uint32_t i = 0; i < m_swapchain.swapchainImageViews.size(); i++)
	{
		vkDestroyImageView(m_renderDevice.logicalDevice, m_swapchain.swapchainImageViews[i], nullptr);

		info.image = m_swapchain.swapchainImages[i];

		vkCreateImageView(m_renderDevice.logicalDevice, &info, nullptr, &m_swapchain.swapchainImageViews[i]);

	}

	VkFramebufferCreateInfo fb_info = {};
	fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fb_info.pNext = nullptr;

	fb_info.renderPass = m_swapchain.renderPass;
	fb_info.attachmentCount = 1;
	fb_info.width = m_instance.surfaceExtent.width;
	fb_info.height = m_instance.surfaceExtent.height;
	fb_info.layers = 1;


	for (uint32_t i = 0; i < m_swapchain.frameBuffers.size(); i++)
	{
		vkDestroyFramebuffer(m_renderDevice.logicalDevice, m_swapchain.frameBuffers[i], nullptr);
		fb_info.pAttachments = &(m_swapchain.swapchainImageViews[i]);
		if (vkCreateFramebuffer(m_renderDevice.logicalDevice, &fb_info, nullptr, &m_swapchain.frameBuffers[i]) != VK_SUCCESS)
		{
			return false;
		}
	}

	swapchain_needs_validate = false;
	return true;
}

bool RenderDevice::init_renderpass()
{
	// Create Render Pass

// the renderpass will use this color attachment.
	VkAttachmentDescription color_attachment = {};
	//the attachment will have the format needed by the swapchain
	color_attachment.format = m_instance.format.format;
	//1 sample, we won't be doing MSAA
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	// we Clear when this attachment is loaded
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// we keep the attachment stored when the renderpass ends
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	//we don't care about stencil
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	//we don't know or care about the starting layout of the attachment
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	//after the renderpass ends, the image has to be on a layout ready for display
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


	VkAttachmentReference color_attachment_ref = {};
	//attachment number will index into the pAttachments array in the parent renderpass itself
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//we are going to create 1 subpass, which is the minimum you can do
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;



	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

	//connect the color attachment to the info
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment;
	//connect the subpass to the info
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;


	if (VK_SUCCESS != vkCreateRenderPass(m_renderDevice.logicalDevice, &render_pass_info, nullptr, &m_swapchain.renderPass))
	{
		return false;
	}

	return true;
}
bool RenderDevice::init_vk_syncs()
{

	VkSemaphoreCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;

	auto res = vkCreateSemaphore(m_renderDevice.logicalDevice, &createInfo, nullptr, &m_renderDevice.imageAcquiredSemaphore);
	if (res != VK_SUCCESS)
		return false;

	res = vkCreateSemaphore(m_renderDevice.logicalDevice, &createInfo, nullptr, &m_renderDevice.renderCompleteSemaphore);
	if (res != VK_SUCCESS)
		return false;

	res = vkCreateSemaphore(m_renderDevice.logicalDevice, &createInfo, nullptr, &m_renderDevice.renderSceneCompleteSemaphore);
	if (res != VK_SUCCESS)
		return false;
	

	VkFenceCreateInfo fCreateInfo = {};
	fCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fCreateInfo.pNext = nullptr;
	fCreateInfo.flags = VkFenceCreateFlagBits::VK_FENCE_CREATE_SIGNALED_BIT; // Signaled

	res = vkCreateFence(m_renderDevice.logicalDevice, &fCreateInfo, nullptr, &m_renderDevice.mainQueueFinishedFence);
	if (res != VK_SUCCESS)
		return false;

	fCreateInfo.flags = 0; // unsignaled
	res = vkCreateFence(m_renderDevice.logicalDevice, &fCreateInfo, nullptr, &m_renderDevice.presentQueueFinishedFence);
	if (res != VK_SUCCESS)
		return false;
	return true;
}

//X TODO : NEED CHANGE COMMAND BUFFERS

VkCommandBuffer RenderDevice::begin_single_time_command()
{

	vkResetCommandPool(m_renderDevice.logicalDevice, m_renderDevice.mainQueueCommandPools[0], 0);
	vkResetFences(m_renderDevice.logicalDevice, 1, &m_renderDevice.presentQueueFinishedFence);
	vkBeginCommandBuffer(m_renderDevice.pMainCommandBuffer, this->get_main_begin_inf());
	return m_renderDevice.pMainCommandBuffer;
}

void RenderDevice::finish_exec_single_time_command(VkCommandBuffer buff)
{
	vkEndCommandBuffer(m_renderDevice.pMainCommandBuffer);
	VkSubmitInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.pNext = nullptr;
	info.commandBufferCount = 1;
	info.pCommandBuffers = &buff;
	info.signalSemaphoreCount = 0;
	info.waitSemaphoreCount = 0;
	vkQueueSubmit(m_renderDevice.mainQueue, 1,&info,nullptr);
	vkQueueWaitIdle(m_renderDevice.mainQueue);
}